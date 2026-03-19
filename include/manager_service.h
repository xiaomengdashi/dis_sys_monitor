#pragma once

#include "metrics.grpc.pb.h"
#include "metrics_types.h"
#include "sqlite_store.h"

#include <mutex>
#include <string>
#include <unordered_map>

namespace dist::sys::monitor {

class ManagerServiceImpl final : public v1::ManagerService::Service {
 public:
  explicit ManagerServiceImpl(const std::string& db_path);

  grpc::Status ReportMetrics(grpc::ServerContext* context,
                             const v1::MetricsBatch* request,
                             v1::Ack* response) override;

  grpc::Status GetBestServer(grpc::ServerContext* context,
                             const v1::BestServerRequest* request,
                             v1::BestServerReply* response) override;

 private:
  MetricSample FromProto(const v1::MetricSample& src) const;

  std::mutex mu_;
  std::unordered_map<std::string, MetricSample> latest_;
  SqliteStore store_;
};

}  // namespace dist::sys::monitor
