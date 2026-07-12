#include "contexts/system_health/application/GetSystemStatus.h"

#include <utility>

namespace kopds::system_health {

GetSystemStatus::GetSystemStatus(
  std::shared_ptr<const HealthCheck> postgresHealthCheck
)
  : postgresHealthCheck_(std::move(postgresHealthCheck))
{
}

std::vector<ComponentHealth> GetSystemStatus::execute() const
{
  return {
    {
      "Application",
      HealthStatus::Healthy,
      "The Wt application is running."
    },
    postgresHealthCheck_->check()
  };
}

} // namespace kopds::system_health
