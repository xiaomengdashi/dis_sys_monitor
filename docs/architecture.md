# 架构设计

## 目标
- 在分布式架构上采集多维系统指标
- 管理者服务器统一记录、统计、分析
- 计算最优工作者节点，降低高负载节点压力
- 监测指标突变，提前预警

## 角色

Agent（工作者）
- 周期性采样
- 采样结果批量上报
- 采样来源可切换：/proc、内核模块、eBPF
- 操作系统：仅 Linux

Manager（管理者）
- gRPC 接收数据
- SQLite 落库与变化率计算
- 计算最优节点并提供查询接口
- 操作系统：Linux/macOS/Windows

## 关键模块

Agent
- 采集模块：`cpu_load`, `cpu_usage`, `softirq`, `memory`, `disk_io`, `net`
- 传输模块：gRPC Stub + Protobuf
- 采样策略：`sample_interval` + `batch_size`

Manager
- 接入模块：gRPC Service
- 存储模块：SQLite Store
- 分析模块：变化率 + 异常识别
- 策略模块：按场景权重评分

## 数据流

1. Agent 每 N 秒采样
2. 组装 MetricsBatch
3. gRPC ReportMetrics
4. Manager 持久化到 SQLite
5. Manager 依据场景返回最优节点

## 采样扩展方案

- 内核模块
  - 采集 CPU idle/iowait/softirq 等高频指标
  - 通过字符设备或 /proc 暴露给用户态

- eBPF
  - kprobe/tracepoint 获取网络收发
  - ring buffer 上送用户态

## 部署

- Docker 镜像封装 Agent/Manager
- Manager 暴露 50051 端口
- Agent 通过环境变量配置 Manager 地址
