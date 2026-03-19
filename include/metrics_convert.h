#pragma once

#include "metrics_types.h"

namespace dist::sys::monitor::v1 {
class MetricSample;
}

namespace dist::sys::monitor {

void FillProto(const MetricSample& src, dist::sys::monitor::v1::MetricSample* dst);

}  // namespace dist::sys::monitor
