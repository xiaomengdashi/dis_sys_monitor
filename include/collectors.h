#pragma once

#include "metrics_types.h"

namespace dist::sys::monitor {

CpuLoad ReadCpuLoad();
CpuUsage ReadCpuUsage();
SoftIrqStats ReadSoftIrqStats();
MemoryStats ReadMemoryStats();
DiskIoStats ReadDiskIoStats();
NetStats ReadNetStats();

}  // namespace dist::sys::monitor
