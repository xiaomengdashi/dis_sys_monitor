#pragma once

#include <atomic>
#include <string>

namespace dist::sys::monitor {

class EbpfNetCollector {
 public:
  static EbpfNetCollector& Instance();

  bool InitFromEnv();
  void Start();
  bool IsReady() const;

  uint64_t RxBytes() const { return rx_bytes_.load(); }
  uint64_t TxBytes() const { return tx_bytes_.load(); }
  void AddRx(uint64_t bytes) { rx_bytes_.fetch_add(bytes); }
  void AddTx(uint64_t bytes) { tx_bytes_.fetch_add(bytes); }

 private:
  EbpfNetCollector();
  bool LoadObject(const std::string& path);

  std::string obj_path_;
  std::atomic<uint64_t> rx_bytes_{0};
  std::atomic<uint64_t> tx_bytes_{0};
  std::atomic<bool> running_{false};
  std::atomic<bool> ready_{false};
};

}  // namespace dist::sys::monitor
