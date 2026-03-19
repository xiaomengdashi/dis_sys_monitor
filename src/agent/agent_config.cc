#include "agent_config.h"

#include <cstdlib>

namespace dist::sys::monitor {

namespace {

std::string GetEnvOrDefault(const char* key, const std::string& fallback) {
  const char* value = std::getenv(key);
  return value ? std::string(value) : fallback;
}

int GetEnvIntOrDefault(const char* key, int fallback) {
  const char* value = std::getenv(key);
  if (!value) {
    return fallback;
  }
  try {
    return std::stoi(value);
  } catch (...) {
    return fallback;
  }
}

}  // namespace

AgentConfig LoadAgentConfig() {
  AgentConfig config;
  config.host_id = GetEnvOrDefault("DSM_HOST_ID", "host-unknown");
  config.manager_address = GetEnvOrDefault("DSM_MANAGER_ADDR", "127.0.0.1:50051");
  config.sample_interval_sec = GetEnvIntOrDefault("DSM_SAMPLE_INTERVAL", 60);
  config.batch_size = GetEnvIntOrDefault("DSM_BATCH_SIZE", 5);
  return config;
}

}  // namespace dist::sys::monitor
