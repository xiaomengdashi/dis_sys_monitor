# eBPF 示例（kprobe + ring buffer）

目标
- 用 kprobe 采集 TCP 发送/接收字节数
- 通过 ring buffer 上送用户态

## 文件

- `src/ebpf/net_kprobe.bpf.c`：eBPF 程序（kprobe + ring buffer）
- `src/ebpf/net_kprobe_user.cpp`：用户态接收示例（libbpf）

## 编译（示例）

依赖
- clang/llvm
- libbpf 开发包（Ubuntu: `libbpf-dev`）
- 内核头文件（`linux-headers-$(uname -r)`）

```bash
# 生成 BPF 对象
clang -O2 -g -target bpf -D__TARGET_ARCH_x86 \
  -I/usr/include \
  -c src/ebpf/net_kprobe.bpf.c -o net_kprobe.bpf.o

# 编译用户态程序
c++ -O2 -g -std=c++20 src/ebpf/net_kprobe_user.cpp -lbpf -lelf -o net_kprobe_user
```

## 运行（示例）

```bash
sudo ./net_kprobe_user ./net_kprobe.bpf.o
```

## 说明

- `tcp_sendmsg` 统计 TX 字节数
- `tcp_cleanup_rbuf` 统计 RX 字节数
- 环境要求：内核 5.4+，开启 `CONFIG_BPF` 与 `CONFIG_BPF_SYSCALL`
- 仅 Linux 支持（agent 本身也是 Linux-only）

## 与 Agent 集成思路

### 已接入的方式（本仓库）

Agent 可在启用 eBPF 编译选项后直接使用该 ring buffer 采集网络字节数：

```bash
cmake -DENABLE_EBPF=ON ..
make -j
```

运行时通过环境变量指定 BPF 对象文件路径：

```bash
DSM_EBPF_OBJ=./net_kprobe.bpf.o ./agent
```

当 `DSM_EBPF_OBJ` 设置且加载成功时，Agent 会优先使用 eBPF 统计网络 RX/TX 字节数与速率；否则回退到 `/proc/net/dev`。

### 其它集成方式

- 用户态程序可将 RX/TX 速率写入共享内存或 gRPC 上报通道
- 也可封装为 Agent 的采集插件，替换 `/proc/net/dev`
