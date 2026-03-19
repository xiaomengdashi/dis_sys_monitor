#include "collectors.h"

#include <fstream>
#include <sstream>
#include <vector>

namespace dist::sys::monitor {

SoftIrqStats ReadSoftIrqStats() {
  SoftIrqStats stats{};
  std::ifstream in("/proc/softirqs");
  if (!in.is_open()) {
    return stats;
  }

  std::string line;
  // Skip header
  std::getline(in, line);

  auto sum_line = [](const std::string& line) -> uint64_t {
    std::istringstream iss(line);
    std::string label;
    iss >> label;
    uint64_t sum = 0;
    uint64_t value = 0;
    while (iss >> value) {
      sum += value;
    }
    return sum;
  };

  std::vector<uint64_t> sums;
  while (std::getline(in, line)) {
    if (line.empty()) {
      continue;
    }
    sums.push_back(sum_line(line));
  }

  if (sums.size() >= 10) {
    stats.hi = sums[0];
    stats.timer = sums[1];
    stats.net_tx = sums[2];
    stats.net_rx = sums[3];
    stats.block = sums[4];
    stats.irq_poll = sums[5];
    stats.tasklet = sums[6];
    stats.sched = sums[7];
    stats.hrtimer = sums[8];
    stats.rcu = sums[9];
  }

  return stats;
}

}  // namespace dist::sys::monitor
