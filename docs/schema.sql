-- SQLite schema
CREATE TABLE IF NOT EXISTS metric_samples (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  host_id TEXT NOT NULL,
  timestamp_ms INTEGER NOT NULL,

  load_1 DOUBLE,
  load_5 DOUBLE,
  load_15 DOUBLE,

  cpu_user_pct DOUBLE,
  cpu_system_pct DOUBLE,
  cpu_idle_pct DOUBLE,
  cpu_iowait_pct DOUBLE,
  cpu_nice_pct DOUBLE,
  cpu_irq_pct DOUBLE,
  cpu_softirq_pct DOUBLE,

  softirq_hi INTEGER,
  softirq_timer INTEGER,
  softirq_net_tx INTEGER,
  softirq_net_rx INTEGER,
  softirq_block INTEGER,
  softirq_irq_poll INTEGER,
  softirq_tasklet INTEGER,
  softirq_sched INTEGER,
  softirq_hrtimer INTEGER,
  softirq_rcu INTEGER,

  mem_total_bytes INTEGER,
  mem_available_bytes INTEGER,
  mem_used_bytes INTEGER,

  disk_read_bytes INTEGER,
  disk_write_bytes INTEGER,
  disk_read_iops DOUBLE,
  disk_write_iops DOUBLE,

  net_rx_bytes INTEGER,
  net_tx_bytes INTEGER,
  net_rx_rate DOUBLE,
  net_tx_rate DOUBLE,

  UNIQUE(host_id, timestamp_ms)
);

CREATE INDEX IF NOT EXISTS idx_host_time ON metric_samples(host_id, timestamp_ms);
