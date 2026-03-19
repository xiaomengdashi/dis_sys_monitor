#include "ebpf_net_collector.h"

#ifdef DSM_ENABLE_EBPF
#include <bpf/libbpf.h>
#include <sys/resource.h>

#include <cstdlib>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

namespace dist::sys::monitor {

namespace {

struct net_event {
  uint64_t ts_ns;
  uint32_t pid;
  uint32_t direction; // 0=rx, 1=tx
  uint64_t bytes;
};

bool BumpMemlock() {
  rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};
  return setrlimit(RLIMIT_MEMLOCK, &rlim) == 0;
}

int HandleEvent(void* ctx, void* data, size_t data_sz) {
  auto* collector = reinterpret_cast<EbpfNetCollector*>(ctx);
  if (data_sz < sizeof(net_event)) {
    return 0;
  }
  const auto* e = reinterpret_cast<const net_event*>(data);
  if (e->direction == 0) {
    collector->AddRx(e->bytes);
  } else {
    collector->AddTx(e->bytes);
  }
  return 0;
}

}  // namespace

EbpfNetCollector::EbpfNetCollector() = default;

EbpfNetCollector& EbpfNetCollector::Instance() {
  static EbpfNetCollector instance;
  return instance;
}

bool EbpfNetCollector::InitFromEnv() {
  const char* path = std::getenv("DSM_EBPF_OBJ");
  if (!path || std::strlen(path) == 0) {
    return false;
  }
  obj_path_ = path;
  return true;
}

bool EbpfNetCollector::IsReady() const {
  return ready_.load();
}

bool EbpfNetCollector::LoadObject(const std::string& path) {
  if (!BumpMemlock()) {
    std::cerr << "[ebpf] failed to increase memlock limit\n";
  }

  libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
  libbpf_set_print([](enum libbpf_print_level level, const char* fmt, va_list args) {
    if (level == LIBBPF_DEBUG) {
      return 0;
    }
    return vfprintf(stderr, fmt, args);
  });

  bpf_object* obj = bpf_object__open_file(path.c_str(), nullptr);
  if (!obj) {
    std::cerr << "[ebpf] open bpf object failed\n";
    return false;
  }

  if (bpf_object__load(obj)) {
    std::cerr << "[ebpf] load bpf object failed\n";
    bpf_object__close(obj);
    return false;
  }

  bpf_program* prog;
  bpf_object__for_each_program(prog, obj) {
    if (!bpf_program__attach(prog)) {
      std::cerr << "[ebpf] attach failed for program: " << bpf_program__name(prog) << "\n";
      bpf_object__close(obj);
      return false;
    }
  }

  bpf_map* events = bpf_object__find_map_by_name(obj, "events");
  if (!events) {
    std::cerr << "[ebpf] events map not found\n";
    bpf_object__close(obj);
    return false;
  }

  int map_fd = bpf_map__fd(events);
  ring_buffer* rb = ring_buffer__new(map_fd, HandleEvent, this, nullptr);
  if (!rb) {
    std::cerr << "[ebpf] ring buffer create failed\n";
    bpf_object__close(obj);
    return false;
  }

  ready_ = true;
  running_ = true;

  std::thread([this, rb, obj]() {
    while (running_.load()) {
      int err = ring_buffer__poll(rb, 200);
      if (err < 0 && err != -EINTR) {
        std::cerr << "[ebpf] ring buffer poll error: " << err << "\n";
        break;
      }
    }
    ring_buffer__free(rb);
    bpf_object__close(obj);
  }).detach();

  return true;
}

void EbpfNetCollector::Start() {
  if (ready_.load() || obj_path_.empty()) {
    return;
  }
  LoadObject(obj_path_);
}

}  // namespace dist::sys::monitor

#else

namespace dist::sys::monitor {

EbpfNetCollector::EbpfNetCollector() = default;
EbpfNetCollector& EbpfNetCollector::Instance() {
  static EbpfNetCollector instance;
  return instance;
}

bool EbpfNetCollector::InitFromEnv() { return false; }
void EbpfNetCollector::Start() {}
bool EbpfNetCollector::IsReady() const { return false; }

}  // namespace dist::sys::monitor

#endif
