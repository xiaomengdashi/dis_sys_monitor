#pragma once

#include <string>

namespace dist::sys::monitor {

struct AgentConfig {
  std::string host_id;
  std::string manager_address;
  int sample_interval_sec{60};
  int batch_size{5};
};

AgentConfig LoadAgentConfig();

}  // namespace dist::sys::monitor
