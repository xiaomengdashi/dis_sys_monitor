#pragma once

#include "metrics_types.h"

#include <string>

struct sqlite3;

namespace dist::sys::monitor {

class SqliteStore {
 public:
  explicit SqliteStore(const std::string& db_path);
  ~SqliteStore();

  bool Connect();
  bool InsertSample(const MetricSample& sample);

 private:
  bool EnsureSchema();

  std::string db_path_;
  sqlite3* db_{nullptr};
};

}  // namespace dist::sys::monitor
