#include "sqlite_store.h"

#include <sqlite3.h>

#include <iostream>

namespace dist::sys::monitor {

SqliteStore::SqliteStore(const std::string& db_path) : db_path_(db_path) {}

SqliteStore::~SqliteStore() {
  if (db_) {
    sqlite3_close(db_);
    db_ = nullptr;
  }
}

bool SqliteStore::Connect() {
  if (sqlite3_open(db_path_.c_str(), &db_) != SQLITE_OK) {
    std::cerr << "[sqlite] open failed: " << sqlite3_errmsg(db_) << "\n";
    return false;
  }
  return EnsureSchema();
}

bool SqliteStore::EnsureSchema() {
  const char* ddl =
      "CREATE TABLE IF NOT EXISTS metric_samples ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "host_id TEXT NOT NULL,"
      "timestamp_ms INTEGER NOT NULL,"
      "load_1 REAL, load_5 REAL, load_15 REAL,"
      "cpu_user_pct REAL, cpu_system_pct REAL, cpu_idle_pct REAL, cpu_iowait_pct REAL,"
      "cpu_nice_pct REAL, cpu_irq_pct REAL, cpu_softirq_pct REAL,"
      "softirq_hi INTEGER, softirq_timer INTEGER, softirq_net_tx INTEGER, softirq_net_rx INTEGER,"
      "softirq_block INTEGER, softirq_irq_poll INTEGER, softirq_tasklet INTEGER, softirq_sched INTEGER,"
      "softirq_hrtimer INTEGER, softirq_rcu INTEGER,"
      "mem_total_bytes INTEGER, mem_available_bytes INTEGER, mem_used_bytes INTEGER,"
      "disk_read_bytes INTEGER, disk_write_bytes INTEGER, disk_read_iops REAL, disk_write_iops REAL,"
      "net_rx_bytes INTEGER, net_tx_bytes INTEGER, net_rx_rate REAL, net_tx_rate REAL"
      ");"
      "CREATE INDEX IF NOT EXISTS idx_host_time ON metric_samples(host_id, timestamp_ms);";

  char* err = nullptr;
  if (sqlite3_exec(db_, ddl, nullptr, nullptr, &err) != SQLITE_OK) {
    std::cerr << "[sqlite] schema failed: " << (err ? err : "unknown") << "\n";
    sqlite3_free(err);
    return false;
  }
  return true;
}

bool SqliteStore::InsertSample(const MetricSample& s) {
  const char* sql =
      "INSERT INTO metric_samples ("
      "host_id, timestamp_ms, load_1, load_5, load_15,"
      "cpu_user_pct, cpu_system_pct, cpu_idle_pct, cpu_iowait_pct, cpu_nice_pct, cpu_irq_pct, cpu_softirq_pct,"
      "softirq_hi, softirq_timer, softirq_net_tx, softirq_net_rx, softirq_block, softirq_irq_poll,"
      "softirq_tasklet, softirq_sched, softirq_hrtimer, softirq_rcu,"
      "mem_total_bytes, mem_available_bytes, mem_used_bytes,"
      "disk_read_bytes, disk_write_bytes, disk_read_iops, disk_write_iops,"
      "net_rx_bytes, net_tx_bytes, net_rx_rate, net_tx_rate"
      ") VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "[sqlite] prepare failed: " << sqlite3_errmsg(db_) << "\n";
    return false;
  }

  int idx = 1;
  sqlite3_bind_text(stmt, idx++, s.host_id.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.timestamp_ms));
  sqlite3_bind_double(stmt, idx++, s.cpu_load.load_1);
  sqlite3_bind_double(stmt, idx++, s.cpu_load.load_5);
  sqlite3_bind_double(stmt, idx++, s.cpu_load.load_15);

  sqlite3_bind_double(stmt, idx++, s.cpu_usage.user_pct);
  sqlite3_bind_double(stmt, idx++, s.cpu_usage.system_pct);
  sqlite3_bind_double(stmt, idx++, s.cpu_usage.idle_pct);
  sqlite3_bind_double(stmt, idx++, s.cpu_usage.iowait_pct);
  sqlite3_bind_double(stmt, idx++, s.cpu_usage.nice_pct);
  sqlite3_bind_double(stmt, idx++, s.cpu_usage.irq_pct);
  sqlite3_bind_double(stmt, idx++, s.cpu_usage.softirq_pct);

  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.hi));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.timer));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.net_tx));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.net_rx));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.block));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.irq_poll));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.tasklet));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.sched));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.hrtimer));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.softirq.rcu));

  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.memory.total_bytes));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.memory.available_bytes));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.memory.used_bytes));

  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.disk_io.read_bytes));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.disk_io.write_bytes));
  sqlite3_bind_double(stmt, idx++, s.disk_io.read_iops);
  sqlite3_bind_double(stmt, idx++, s.disk_io.write_iops);

  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.net.rx_bytes));
  sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(s.net.tx_bytes));
  sqlite3_bind_double(stmt, idx++, s.net.rx_rate);
  sqlite3_bind_double(stmt, idx++, s.net.tx_rate);

  bool ok = true;
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    std::cerr << "[sqlite] insert failed: " << sqlite3_errmsg(db_) << "\n";
    ok = false;
  }
  sqlite3_finalize(stmt);
  return ok;
}

}  // namespace dist::sys::monitor
