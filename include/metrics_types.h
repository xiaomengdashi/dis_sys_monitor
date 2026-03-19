#pragma once

#include <cstdint>
#include <string>

namespace dist::sys::monitor {

struct CpuLoad {
  double load_1{};
  double load_5{};
  double load_15{};
};

struct CpuUsage {
  double user_pct{};
  double system_pct{};
  double idle_pct{};
  double iowait_pct{};
  double nice_pct{};
  double irq_pct{};
  double softirq_pct{};
};

struct SoftIrqStats {
  uint64_t hi{};
  uint64_t timer{};
  uint64_t net_tx{};
  uint64_t net_rx{};
  uint64_t block{};
  uint64_t irq_poll{};
  uint64_t tasklet{};
  uint64_t sched{};
  uint64_t hrtimer{};
  uint64_t rcu{};
};

struct MemoryStats {
  uint64_t total_bytes{};
  uint64_t available_bytes{};
  uint64_t used_bytes{};
};

struct DiskIoStats {
  uint64_t read_bytes{};
  uint64_t write_bytes{};
  double read_iops{};
  double write_iops{};
};

struct NetStats {
  uint64_t rx_bytes{};
  uint64_t tx_bytes{};
  double rx_rate{};
  double tx_rate{};
};

struct MetricSample {
  std::string host_id;
  uint64_t timestamp_ms{};
  CpuLoad cpu_load;
  CpuUsage cpu_usage;
  SoftIrqStats softirq;
  MemoryStats memory;
  DiskIoStats disk_io;
  NetStats net;
};

}  // namespace dist::sys::monitor
