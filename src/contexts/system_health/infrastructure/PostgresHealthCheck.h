#pragma once

#include <memory>

#include "contexts/system_health/application/HealthCheck.h"
#include "shared/infrastructure/Database.h"

namespace kopds::system_health {

class PostgresHealthCheck final : public HealthCheck {
public:
  explicit PostgresHealthCheck(std::shared_ptr<const shared::Database> database);

  ComponentHealth check() const override;

private:
  std::shared_ptr<const shared::Database> database_;
};

} // namespace kopds::system_health
