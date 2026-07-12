#pragma once

#include "contexts/system_health/domain/ComponentHealth.h"

namespace kopds::system_health {

class HealthCheck {
public:
  virtual ~HealthCheck() = default;

  virtual ComponentHealth check() const = 0;
};

} // namespace kopds::system_health
