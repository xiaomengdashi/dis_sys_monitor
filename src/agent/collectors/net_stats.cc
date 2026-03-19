#include "collectors.h"

#include <chrono>
#include <fstream>
#include <sstream>

#ifdef DSM_ENABLE_EBPF
#include "ebpf_net_collector.h"
#endif

namespace dist::sys::monitor {

NetStats ReadNetStats() {
  NetStats stats{};

#ifdef DSM_ENABLE_EBPF
  auto& ebpf = EbpfNetCollector::Instance();
  if (ebpf.IsReady()) {
    const uint64_t rx_bytes = ebpf.RxBytes();
    const uint64_t tx_bytes = ebpf.TxBytes();

    static bool has_prev = false;
    static uint64_t prev_rx = 0;
    static uint64_t prev_tx = 0;
    static auto prev_time = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();
    double delta_sec = std::chrono::duration<double>(now - prev_time).count();

    stats.rx_bytes = rx_bytes;
    stats.tx_bytes = tx_bytes;

    if (has_prev && delta_sec > 0.0) {
      stats.rx_rate = static_cast<double>(rx_bytes - prev_rx) / delta_sec;
      stats.tx_rate = static_cast<double>(tx_bytes - prev_tx) / delta_sec;
    }

    prev_rx = rx_bytes;
    prev_tx = tx_bytes;
    prev_time = now;
    has_prev = true;

    return stats;
  }
#endif

  std::ifstream in("/proc/net/dev");
  if (!in.is_open()) {
    return stats;
  }

  std::string line;
  // Skip headers
  std::getline(in, line);
  std::getline(in, line);

  uint64_t rx_bytes = 0;
  uint64_t tx_bytes = 0;

  while (std::getline(in, line)) {
    if (line.empty()) {
      continue;
    }
    std::istringstream iss(line);
    std::string iface;
    iss >> iface;
    if (iface.find("lo:") == 0) {
      continue;
    }

    uint64_t rx = 0, rx_packets = 0, rx_err = 0, rx_drop = 0, rx_fifo = 0, rx_frame = 0, rx_compressed = 0, rx_multicast = 0;
    uint64_t tx = 0, tx_packets = 0, tx_err = 0, tx_drop = 0, tx_fifo = 0, tx_colls = 0, tx_carrier = 0, tx_compressed = 0;

    iss >> rx >> rx_packets >> rx_err >> rx_drop >> rx_fifo >> rx_frame >> rx_compressed >> rx_multicast
        >> tx >> tx_packets >> tx_err >> tx_drop >> tx_fifo >> tx_colls >> tx_carrier >> tx_compressed;

    rx_bytes += rx;
    tx_bytes += tx;
  }

  static bool has_prev = false;
  static uint64_t prev_rx = 0;
  static uint64_t prev_tx = 0;
  static auto prev_time = std::chrono::steady_clock::now();

  auto now = std::chrono::steady_clock::now();
  double delta_sec = std::chrono::duration<double>(now - prev_time).count();

  stats.rx_bytes = rx_bytes;
  stats.tx_bytes = tx_bytes;

  if (has_prev && delta_sec > 0.0) {
    stats.rx_rate = static_cast<double>(rx_bytes - prev_rx) / delta_sec;
    stats.tx_rate = static_cast<double>(tx_bytes - prev_tx) / delta_sec;
  }

  prev_rx = rx_bytes;
  prev_tx = tx_bytes;
  prev_time = now;
  has_prev = true;

  return stats;
}

}  // namespace dist::sys::monitor
