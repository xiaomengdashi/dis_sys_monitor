#include "collectors.h"

#include <chrono>
#include <fstream>
#include <sstream>
#include <string>

namespace dist::sys::monitor {

MemoryStats ReadMemoryStats() {
  MemoryStats stats{};
  std::ifstream in("/proc/meminfo");
  if (!in.is_open()) {
    return stats;
  }

  std::string key;
  uint64_t value_kb = 0;
  std::string unit;
  uint64_t total_kb = 0;
  uint64_t available_kb = 0;
  while (in >> key >> value_kb >> unit) {
    if (key == "MemTotal:") {
      total_kb = value_kb;
    } else if (key == "MemAvailable:") {
      available_kb = value_kb;
    }
  }

  stats.total_bytes = total_kb * 1024;
  stats.available_bytes = available_kb * 1024;
  stats.used_bytes = (total_kb - available_kb) * 1024;
  return stats;
}

DiskIoStats ReadDiskIoStats() {
  DiskIoStats stats{};
  std::ifstream in("/proc/diskstats");
  if (!in.is_open()) {
    return stats;
  }

  uint64_t reads_completed = 0;
  uint64_t writes_completed = 0;
  uint64_t sectors_read = 0;
  uint64_t sectors_written = 0;

  std::string line;
  while (std::getline(in, line)) {
    if (line.empty()) {
      continue;
    }
    std::istringstream iss(line);
    int major = 0;
    int minor = 0;
    std::string device;
    uint64_t reads = 0, reads_merged = 0, read_sectors = 0, read_ms = 0;
    uint64_t writes = 0, writes_merged = 0, write_sectors = 0, write_ms = 0;

    iss >> major >> minor >> device;
    if (device.find("loop") == 0 || device.find("ram") == 0) {
      continue;
    }

    iss >> reads >> reads_merged >> read_sectors >> read_ms
        >> writes >> writes_merged >> write_sectors >> write_ms;

    reads_completed += reads;
    writes_completed += writes;
    sectors_read += read_sectors;
    sectors_written += write_sectors;
  }

  static bool has_prev = false;
  static uint64_t prev_reads = 0;
  static uint64_t prev_writes = 0;
  static uint64_t prev_sectors_read = 0;
  static uint64_t prev_sectors_written = 0;
  static auto prev_time = std::chrono::steady_clock::now();

  auto now = std::chrono::steady_clock::now();
  double delta_sec = std::chrono::duration<double>(now - prev_time).count();

  const uint64_t cur_read_bytes = sectors_read * 512;
  const uint64_t cur_write_bytes = sectors_written * 512;

  stats.read_bytes = cur_read_bytes;
  stats.write_bytes = cur_write_bytes;

  if (has_prev && delta_sec > 0.0) {
    stats.read_iops = static_cast<double>(reads_completed - prev_reads) / delta_sec;
    stats.write_iops = static_cast<double>(writes_completed - prev_writes) / delta_sec;
  }

  prev_reads = reads_completed;
  prev_writes = writes_completed;
  prev_sectors_read = sectors_read;
  prev_sectors_written = sectors_written;
  prev_time = now;
  has_prev = true;

  (void)prev_sectors_read;
  (void)prev_sectors_written;
  return stats;
}

}  // namespace dist::sys::monitor
