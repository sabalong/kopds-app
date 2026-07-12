#pragma once

#include <string>

namespace kopds::system_health {

enum class HealthStatus {
  Healthy,
  Unavailable,
  NotConfigured
};

struct ComponentHealth {
  std::string component;
  HealthStatus status;
  std::string message;
};

} // namespace kopds::system_health
