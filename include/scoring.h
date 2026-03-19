#pragma once

#include "metrics_types.h"

#include <string>

namespace dist::sys::monitor {

struct ScoreResult {
  std::string host_id;
  double score{};
};

ScoreResult ScoreHost(const MetricSample& sample, const std::string& scenario);

}  // namespace dist::sys::monitor
