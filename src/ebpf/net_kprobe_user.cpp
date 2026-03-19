#include <bpf/libbpf.h>
#include <sys/resource.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>

namespace {

struct net_event {
  uint64_t ts_ns;
  uint32_t pid;
  uint32_t direction; // 0=rx, 1=tx
  uint64_t bytes;
};

std::atomic<bool> g_running{true};

void HandleSignal(int) {
  g_running = false;
}

int HandleEvent(void* ctx, void* data, size_t data_sz) {
  (void)ctx;
  if (data_sz < sizeof(net_event)) {
    return 0;
  }
  const auto* e = reinterpret_cast<const net_event*>(data);
  const char* dir = e->direction == 0 ? "rx" : "tx";
  std::cout << "event dir=" << dir << " bytes=" << e->bytes << " pid=" << e->pid << "\n";
  return 0;
}

bool BumpMemlock() {
  rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};
  return setrlimit(RLIMIT_MEMLOCK, &rlim) == 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (!BumpMemlock()) {
    std::cerr << "failed to increase memlock limit\n";
    return 1;
  }

  const char* obj_path = argc > 1 ? argv[1] : "net_kprobe.bpf.o";

  libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
  libbpf_set_print([](enum libbpf_print_level level, const char* fmt, va_list args) {
    if (level == LIBBPF_DEBUG) {
      return 0;
    }
    return vfprintf(stderr, fmt, args);
  });

  bpf_object* obj = bpf_object__open_file(obj_path, nullptr);
  if (!obj) {
    std::cerr << "open bpf object failed\n";
    return 1;
  }

  if (bpf_object__load(obj)) {
    std::cerr << "load bpf object failed\n";
    bpf_object__close(obj);
    return 1;
  }

  bpf_program* prog;
  bpf_object__for_each_program(prog, obj) {
    if (!bpf_program__attach(prog)) {
      std::cerr << "attach failed for program: " << bpf_program__name(prog) << "\n";
      bpf_object__close(obj);
      return 1;
    }
  }

  bpf_map* events = bpf_object__find_map_by_name(obj, "events");
  if (!events) {
    std::cerr << "events map not found\n";
    bpf_object__close(obj);
    return 1;
  }

  int map_fd = bpf_map__fd(events);
  ring_buffer* rb = ring_buffer__new(map_fd, HandleEvent, nullptr, nullptr);
  if (!rb) {
    std::cerr << "ring buffer create failed\n";
    bpf_object__close(obj);
    return 1;
  }

  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);

  while (g_running.load()) {
    int err = ring_buffer__poll(rb, 200 /* ms */);
    if (err < 0 && err != -EINTR) {
      std::cerr << "ring buffer poll error: " << err << "\n";
      break;
    }
  }

  ring_buffer__free(rb);
  bpf_object__close(obj);
  return 0;
}
