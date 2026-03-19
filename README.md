# 分布式架构系统性能监控分析

面向分布式架构的系统性能监控与分析。工作者服务器采集 CPU/内存/磁盘/网络等指标，通过 gRPC + Protobuf 上报至管理者服务器进行记录、统计、异常识别与最优节点选择。

## 架构概览

组件
- Agent（工作者服务器）：采集指标、打包上报
- Manager（管理者服务器）：接收数据、落库、计算指标变化率、计算最优服务器
- SQLite：指标存储与统计
- eBPF/内核模块：更高精度指标来源

数据流
1. Agent 周期性采样
2. gRPC 上报 MetricsBatch
3. Manager 接收并写入 SQLite
4. Manager 基于最新指标进行评分，返回最优节点

## 平台支持

| 组件 | Linux | macOS | Windows |
|---|---|---|---|
| manager | 支持 | 支持 | 支持 |
| agent | 支持 | 不支持 | 不支持 |

说明
- `manager` 依赖 gRPC/Protobuf/SQLite，支持跨平台部署。
- `agent` 依赖 Linux `/proc` 与可选 eBPF 采集链路，仅支持 Linux。

## 模块设计

采集层（Agent）
- `cpu_load`: /proc/loadavg
- `cpu_usage`: /proc/stat（可替换为内核模块采样）
- `softirq`: /proc/softirqs（可替换为内核模块）
- `memory`: /proc/meminfo
- `disk_io`: /proc/diskstats
- `net`: /proc/net/dev（可替换为 eBPF）

通信层
- `proto/metrics.proto`: 指标模型与 `ManagerService`

管理层（Manager）
- 入库：SQLite（本地轻量存储）
- 评分：按场景权重计算最优节点

## 关键指标与评分

评分使用可配置权重（示例实现包含 `cpu-heavy`/`io-heavy`/`net-heavy` 三类场景）。
- CPU：使用率/空闲率
- Memory：可用内存比例
- IO：IOPS 压力
- Network：吞吐压力

## 目录结构

- `proto/` Protobuf 与 gRPC 接口
- `src/agent/` 采集与上报
- `src/manager/` 指标接收、评分与落库
- `include/` 公共类型
- `docs/architecture.md` 架构设计
- `docs/schema.sql` SQLite 表结构
- `docs/ebpf.md` eBPF 示例说明
- `docs/build.md` 跨平台编译说明
- `scripts/` 构建脚本

## 构建（示例）

依赖：gRPC、Protobuf、SQLite3、CMake

推荐使用脚本：

```bash
# manager（Linux/macOS）
./scripts/build_manager.sh

# agent（Linux only）
./scripts/build_agent.sh
```

## CI

- 工作流文件：`.github/workflows/ci.yml`
- 覆盖：`manager`（Linux/Windows）与 `agent`（Linux）

## 运行（示例）

```bash
# Manager
./manager 0.0.0.0:50051 ./metrics.db

# Agent
DSM_HOST_ID=host-1 DSM_MANAGER_ADDR=127.0.0.1:50051 ./agent
```

如需启用 eBPF 网络采样（需要 libbpf）：

```bash
ENABLE_EBPF=ON ./scripts/build_agent.sh
DSM_EBPF_OBJ=./net_kprobe.bpf.o ./agent
```

## Windows 编译 manager

参考 `docs/build.md`，推荐使用 `vcpkg` + PowerShell 脚本：

```powershell
./scripts/build_manager.ps1 -Config Release
```

## 内核模块与 eBPF（概念设计）

- 内核模块采样：采集 `idle/iowait/softirq` 等高频指标，减少用户态解析开销。
- eBPF 采样：从内核协议栈采集 `rx/tx` 字节数与速率。

该仓库提供 C++ 框架与接口占位，方便后续接入内核模块或 eBPF 程序。

## 简历描述建议（可直接使用）

- 设计并实现分布式性能监控系统，支持 Agent 采集与 Manager 汇总分析，基于 gRPC + Protobuf 低开销传输多维指标
- 基于 SQLite 记录历史指标并计算变化率，实现异常波动检测与最优节点选择
- 结合 /proc 与内核态采样（内核模块/eBPF 预留接口），支持 CPU/内存/磁盘/网络全栈指标
- 基于不同业务场景配置权重，计算最优工作者服务器以实现负载削峰
