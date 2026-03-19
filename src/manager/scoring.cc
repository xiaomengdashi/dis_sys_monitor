#include "scoring.h"

#include <algorithm>

namespace dist::sys::monitor {

namespace {

double Clamp(double v, double lo, double hi) {
  return std::max(lo, std::min(v, hi));
}

}  // namespace

ScoreResult ScoreHost(const MetricSample& sample, const std::string& scenario) {
  const double mem_free_pct = sample.memory.total_bytes > 0
                                  ? (100.0 * static_cast<double>(sample.memory.available_bytes) /
                                     static_cast<double>(sample.memory.total_bytes))
                                  : 0.0;

  const double cpu_idle = sample.cpu_usage.idle_pct;
  const double load_score = 100.0 / (sample.cpu_load.load_1 + 1.0);
  const double io_pressure = sample.disk_io.read_iops + sample.disk_io.write_iops + 1.0;
  const double net_pressure = sample.net.rx_rate + sample.net.tx_rate + 1.0;

  const double io_score = 100.0 / io_pressure;
  const double net_score = 100.0 / net_pressure;

  double w_cpu = 0.4;
  double w_mem = 0.3;
  double w_io = 0.2;
  double w_net = 0.1;

  if (scenario == "cpu-heavy") {
    w_cpu = 0.6;
    w_mem = 0.2;
    w_io = 0.15;
    w_net = 0.05;
  } else if (scenario == "io-heavy") {
    w_cpu = 0.25;
    w_mem = 0.25;
    w_io = 0.4;
    w_net = 0.1;
  } else if (scenario == "net-heavy") {
    w_cpu = 0.25;
    w_mem = 0.2;
    w_io = 0.15;
    w_net = 0.4;
  }

  const double score = w_cpu * Clamp(cpu_idle, 0.0, 100.0) +
                       w_mem * Clamp(mem_free_pct, 0.0, 100.0) +
                       w_io * Clamp(io_score, 0.0, 100.0) +
                       w_net * Clamp(net_score, 0.0, 100.0) +
                       0.1 * Clamp(load_score, 0.0, 100.0);

  return ScoreResult{sample.host_id, score};
}

}  // namespace dist::sys::monitor
