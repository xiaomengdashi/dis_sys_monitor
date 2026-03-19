# 编译与平台支持

## 平台支持矩阵

| 组件 | Linux | macOS | Windows |
|---|---|---|---|
| manager | 支持 | 支持 | 支持 |
| agent | 支持 | 不支持 | 不支持 |

说明
- `manager` 依赖跨平台库（gRPC/Protobuf/SQLite），支持 Linux/macOS/Windows。
- `agent` 当前采集链路依赖 Linux `/proc` 与可选 eBPF，因此仅支持 Linux。

## CMake 开关

- `-DBUILD_MANAGER=ON/OFF`：是否编译 `manager`。
- `-DBUILD_AGENT=ON/OFF`：是否编译 `agent`（仅 Linux 允许 ON）。
- `-DENABLE_EBPF=ON/OFF`：是否启用 eBPF 网络采集（仅 Linux）。

## 构建脚本

- Linux/macOS 构建 manager：`scripts/build_manager.sh`
- Linux 构建 agent：`scripts/build_agent.sh`
- Windows 构建 manager：`scripts/build_manager.ps1`

## manager 编译

### macOS

依赖：`cmake protobuf grpc sqlite3`

```bash
brew install cmake protobuf grpc sqlite3
./scripts/build_manager.sh
```

产物：`build/manager/manager`

### Linux

依赖（Ubuntu 22.04 示例）

```bash
sudo apt update
sudo apt install -y build-essential cmake protobuf-compiler libprotobuf-dev \
  libgrpc++-dev protobuf-compiler-grpc libsqlite3-dev
./scripts/build_manager.sh
```

产物：`build/manager/manager`

### Windows（推荐 vcpkg）

依赖安装（PowerShell）

```powershell
vcpkg install grpc protobuf sqlite3
```

构建

```powershell
./scripts/build_manager.ps1 -Config Release -Generator "Visual Studio 17 2022" -Architecture x64
```

如果未设置 `VCPKG_ROOT`，可显式指定：

```powershell
./scripts/build_manager.ps1 -Config Release -VcpkgToolchain "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

产物：`build/manager-win/Release/manager.exe`（或所选生成器对应目录）

## agent 编译（仅 Linux）

依赖（Ubuntu 22.04 示例）

```bash
sudo apt update
sudo apt install -y build-essential cmake protobuf-compiler libprotobuf-dev \
  libgrpc++-dev protobuf-compiler-grpc libsqlite3-dev
```

构建

```bash
./scripts/build_agent.sh
```

产物：`build/agent/agent`

开启 eBPF 构建

```bash
ENABLE_EBPF=ON ./scripts/build_agent.sh
```

额外依赖：`libbpf-dev clang llvm linux-headers-$(uname -r)`

## 手动编译（不使用脚本）

manager：

```bash
cmake -S . -B build/manager -DBUILD_MANAGER=ON -DBUILD_AGENT=OFF
cmake --build build/manager --target manager -j
```

agent（Linux）：

```bash
cmake -S . -B build/agent -DBUILD_MANAGER=OFF -DBUILD_AGENT=ON
cmake --build build/agent --target agent -j
```

## CI（GitHub Actions）

仓库内置工作流：`.github/workflows/ci.yml`

覆盖范围
- `manager` Linux 构建
- `manager` Windows 构建
- `agent` Linux 构建

触发条件
- push
- pull_request

说明
- Windows 任务使用 `vcpkg` 安装 `grpc/protobuf/sqlite3`。
- Linux 任务使用 `apt` 安装依赖后调用仓库脚本构建。
