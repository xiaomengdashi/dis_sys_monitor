#include "collectors.h"

#include <fstream>
#include <sstream>

namespace dist::sys::monitor {

namespace {

struct CpuTimes {
  uint64_t user{};
  uint64_t nice{};
  uint64_t system{};
  uint64_t idle{};
  uint64_t iowait{};
  uint64_t irq{};
  uint64_t softirq{};
};

bool ReadCpuTimes(CpuTimes* out) {
  std::ifstream in("/proc/stat");
  if (!in.is_open()) {
    return false;
  }
  std::string line;
  std::getline(in, line);
  std::istringstream iss(line);
  std::string cpu_label;
  iss >> cpu_label;
  if (cpu_label != "cpu") {
    return false;
  }
  iss >> out->user >> out->nice >> out->system >> out->idle >> out->iowait >> out->irq >> out->softirq;
  return true;
}

}  // namespace

CpuUsage ReadCpuUsage() {
  static bool has_prev = false;
  static CpuTimes prev{};

  CpuTimes cur{};
  CpuUsage usage{};
  if (!ReadCpuTimes(&cur)) {
    return usage;
  }

  if (!has_prev) {
    prev = cur;
    has_prev = true;
    return usage;
  }

  const uint64_t prev_total = prev.user + prev.nice + prev.system + prev.idle + prev.iowait + prev.irq + prev.softirq;
  const uint64_t cur_total = cur.user + cur.nice + cur.system + cur.idle + cur.iowait + cur.irq + cur.softirq;
  const uint64_t delta_total = cur_total - prev_total;
  if (delta_total == 0) {
    return usage;
  }

  auto pct = [delta_total](uint64_t cur_v, uint64_t prev_v) {
    return 100.0 * static_cast<double>(cur_v - prev_v) / static_cast<double>(delta_total);
  };

  usage.user_pct = pct(cur.user, prev.user);
  usage.nice_pct = pct(cur.nice, prev.nice);
  usage.system_pct = pct(cur.system, prev.system);
  usage.idle_pct = pct(cur.idle, prev.idle);
  usage.iowait_pct = pct(cur.iowait, prev.iowait);
  usage.irq_pct = pct(cur.irq, prev.irq);
  usage.softirq_pct = pct(cur.softirq, prev.softirq);

  prev = cur;
  return usage;
}

}  // namespace dist::sys::monitor
