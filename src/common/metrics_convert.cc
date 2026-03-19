#include "metrics_types.h"

#include "metrics.pb.h"

namespace dist::sys::monitor {

void FillProto(const MetricSample& src, dist::sys::monitor::v1::MetricSample* dst) {
  dst->set_host_id(src.host_id);
  dst->set_timestamp_ms(src.timestamp_ms);

  auto* load = dst->mutable_cpu_load();
  load->set_load_1(src.cpu_load.load_1);
  load->set_load_5(src.cpu_load.load_5);
  load->set_load_15(src.cpu_load.load_15);

  auto* usage = dst->mutable_cpu_usage();
  usage->set_user_pct(src.cpu_usage.user_pct);
  usage->set_system_pct(src.cpu_usage.system_pct);
  usage->set_idle_pct(src.cpu_usage.idle_pct);
  usage->set_iowait_pct(src.cpu_usage.iowait_pct);
  usage->set_nice_pct(src.cpu_usage.nice_pct);
  usage->set_irq_pct(src.cpu_usage.irq_pct);
  usage->set_softirq_pct(src.cpu_usage.softirq_pct);

  auto* softirq = dst->mutable_softirq();
  softirq->set_hi(src.softirq.hi);
  softirq->set_timer(src.softirq.timer);
  softirq->set_net_tx(src.softirq.net_tx);
  softirq->set_net_rx(src.softirq.net_rx);
  softirq->set_block(src.softirq.block);
  softirq->set_irq_poll(src.softirq.irq_poll);
  softirq->set_tasklet(src.softirq.tasklet);
  softirq->set_sched(src.softirq.sched);
  softirq->set_hrtimer(src.softirq.hrtimer);
  softirq->set_rcu(src.softirq.rcu);

  auto* mem = dst->mutable_memory();
  mem->set_total_bytes(src.memory.total_bytes);
  mem->set_available_bytes(src.memory.available_bytes);
  mem->set_used_bytes(src.memory.used_bytes);

  auto* disk = dst->mutable_disk_io();
  disk->set_read_bytes(src.disk_io.read_bytes);
  disk->set_write_bytes(src.disk_io.write_bytes);
  disk->set_read_iops(src.disk_io.read_iops);
  disk->set_write_iops(src.disk_io.write_iops);

  auto* net = dst->mutable_net();
  net->set_rx_bytes(src.net.rx_bytes);
  net->set_tx_bytes(src.net.tx_bytes);
  net->set_rx_rate(src.net.rx_rate);
  net->set_tx_rate(src.net.tx_rate);
}

}  // namespace dist::sys::monitor
