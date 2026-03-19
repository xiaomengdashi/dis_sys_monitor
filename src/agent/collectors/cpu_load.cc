#include "collectors.h"

#include <fstream>
#include <sstream>

namespace dist::sys::monitor {

CpuLoad ReadCpuLoad() {
  CpuLoad load;
  std::ifstream in("/proc/loadavg");
  if (!in.is_open()) {
    return load;
  }
  std::string line;
  std::getline(in, line);
  std::istringstream iss(line);
  iss >> load.load_1 >> load.load_5 >> load.load_15;
  return load;
}

}  // namespace dist::sys::monitor
