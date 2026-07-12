#pragma once

#include <memory>
#include <vector>

#include "contexts/system_health/application/HealthCheck.h"
#include "contexts/system_health/domain/ComponentHealth.h"

namespace kopds::system_health {

class GetSystemStatus {
public:
  explicit GetSystemStatus(std::shared_ptr<const HealthCheck> postgresHealthCheck);

  std::vector<ComponentHealth> execute() const;

private:
  std::shared_ptr<const HealthCheck> postgresHealthCheck_;
};

} // namespace kopds::system_health
