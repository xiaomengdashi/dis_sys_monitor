#include "agent_config.h"
#include "collectors.h"
#include "metrics_convert.h"
#include "metrics_types.h"

#include "metrics.grpc.pb.h"

#include <grpcpp/grpcpp.h>

#include <chrono>
#include <iostream>
#include <thread>

#ifdef DSM_ENABLE_EBPF
#include "ebpf_net_collector.h"
#endif

namespace dist::sys::monitor {

namespace {

uint64_t NowMs() {
  auto now = std::chrono::system_clock::now();
  return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
}

MetricSample CollectOne(const std::string& host_id) {
  MetricSample sample{};
  sample.host_id = host_id;
  sample.timestamp_ms = NowMs();
  sample.cpu_load = ReadCpuLoad();
  sample.cpu_usage = ReadCpuUsage();
  sample.softirq = ReadSoftIrqStats();
  sample.memory = ReadMemoryStats();
  sample.disk_io = ReadDiskIoStats();
  sample.net = ReadNetStats();
  return sample;
}

}  // namespace

}  // namespace dist::sys::monitor

int main() {
  using dist::sys::monitor::AgentConfig;
  using dist::sys::monitor::CollectOne;
  using dist::sys::monitor::FillProto;

  AgentConfig config = dist::sys::monitor::LoadAgentConfig();

#ifdef DSM_ENABLE_EBPF
  auto& ebpf = dist::sys::monitor::EbpfNetCollector::Instance();
  ebpf.InitFromEnv();
  ebpf.Start();
#endif

  auto channel = grpc::CreateChannel(config.manager_address, grpc::InsecureChannelCredentials());
  auto stub = dist::sys::monitor::v1::ManagerService::NewStub(channel);

  dist::sys::monitor::v1::MetricsBatch batch;
  int count = 0;

  while (true) {
    auto sample = CollectOne(config.host_id);

    auto* proto_sample = batch.add_samples();
    FillProto(sample, proto_sample);
    count++;

    if (count >= config.batch_size) {
      grpc::ClientContext ctx;
      dist::sys::monitor::v1::Ack ack;
      auto status = stub->ReportMetrics(&ctx, batch, &ack);
      if (!status.ok()) {
        std::cerr << "ReportMetrics failed: " << status.error_message() << "\n";
      }
      batch.clear_samples();
      count = 0;
    }

    std::this_thread::sleep_for(std::chrono::seconds(config.sample_interval_sec));
  }

  return 0;
}
