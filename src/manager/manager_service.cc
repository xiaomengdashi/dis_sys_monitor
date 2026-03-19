#include "manager_service.h"

#include "scoring.h"

#include <iostream>

namespace dist::sys::monitor {

ManagerServiceImpl::ManagerServiceImpl(const std::string& db_path) : store_(db_path) {
  store_.Connect();
}

MetricSample ManagerServiceImpl::FromProto(const v1::MetricSample& src) const {
  MetricSample dst{};
  dst.host_id = src.host_id();
  dst.timestamp_ms = src.timestamp_ms();

  if (src.has_cpu_load()) {
    dst.cpu_load.load_1 = src.cpu_load().load_1();
    dst.cpu_load.load_5 = src.cpu_load().load_5();
    dst.cpu_load.load_15 = src.cpu_load().load_15();
  }

  if (src.has_cpu_usage()) {
    dst.cpu_usage.user_pct = src.cpu_usage().user_pct();
    dst.cpu_usage.system_pct = src.cpu_usage().system_pct();
    dst.cpu_usage.idle_pct = src.cpu_usage().idle_pct();
    dst.cpu_usage.iowait_pct = src.cpu_usage().iowait_pct();
    dst.cpu_usage.nice_pct = src.cpu_usage().nice_pct();
    dst.cpu_usage.irq_pct = src.cpu_usage().irq_pct();
    dst.cpu_usage.softirq_pct = src.cpu_usage().softirq_pct();
  }

  if (src.has_softirq()) {
    dst.softirq.hi = src.softirq().hi();
    dst.softirq.timer = src.softirq().timer();
    dst.softirq.net_tx = src.softirq().net_tx();
    dst.softirq.net_rx = src.softirq().net_rx();
    dst.softirq.block = src.softirq().block();
    dst.softirq.irq_poll = src.softirq().irq_poll();
    dst.softirq.tasklet = src.softirq().tasklet();
    dst.softirq.sched = src.softirq().sched();
    dst.softirq.hrtimer = src.softirq().hrtimer();
    dst.softirq.rcu = src.softirq().rcu();
  }

  if (src.has_memory()) {
    dst.memory.total_bytes = src.memory().total_bytes();
    dst.memory.available_bytes = src.memory().available_bytes();
    dst.memory.used_bytes = src.memory().used_bytes();
  }

  if (src.has_disk_io()) {
    dst.disk_io.read_bytes = src.disk_io().read_bytes();
    dst.disk_io.write_bytes = src.disk_io().write_bytes();
    dst.disk_io.read_iops = src.disk_io().read_iops();
    dst.disk_io.write_iops = src.disk_io().write_iops();
  }

  if (src.has_net()) {
    dst.net.rx_bytes = src.net().rx_bytes();
    dst.net.tx_bytes = src.net().tx_bytes();
    dst.net.rx_rate = src.net().rx_rate();
    dst.net.tx_rate = src.net().tx_rate();
  }

  return dst;
}

grpc::Status ManagerServiceImpl::ReportMetrics(grpc::ServerContext*,
                                               const v1::MetricsBatch* request,
                                               v1::Ack* response) {
  std::lock_guard<std::mutex> lock(mu_);
  for (const auto& sample : request->samples()) {
    auto data = FromProto(sample);
    latest_[data.host_id] = data;
    store_.InsertSample(data);
  }
  response->set_ok(true);
  response->set_message("ok");
  return grpc::Status::OK;
}

grpc::Status ManagerServiceImpl::GetBestServer(grpc::ServerContext*,
                                               const v1::BestServerRequest* request,
                                               v1::BestServerReply* response) {
  std::lock_guard<std::mutex> lock(mu_);
  double best_score = -1.0;
  std::string best_host;
  for (const auto& [host, sample] : latest_) {
    auto result = ScoreHost(sample, request->scenario());
    if (result.score > best_score) {
      best_score = result.score;
      best_host = host;
    }
  }

  response->set_host_id(best_host);
  response->set_score(best_score);
  return grpc::Status::OK;
}

}  // namespace dist::sys::monitor
