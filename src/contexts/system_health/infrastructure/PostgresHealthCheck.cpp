#include "contexts/system_health/infrastructure/PostgresHealthCheck.h"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <utility>

#include <Wt/Dbo/Dbo.h>

namespace kopds::system_health {

PostgresHealthCheck::PostgresHealthCheck(
  std::shared_ptr<const shared::Database> database
)
  : database_(std::move(database))
{
}

ComponentHealth PostgresHealthCheck::check() const
{
  if (!database_->isConfigured()) {
    return {
      "PostgreSQL",
      HealthStatus::NotConfigured,
      "DATABASE_URL is not configured."
    };
  }

  try {
    auto session = database_->openSession();
    Wt::Dbo::Transaction transaction(*session);
    const int result = session->query<int>("select 1").resultValue();
    transaction.commit();

    if (result != 1) {
      throw std::runtime_error("PostgreSQL returned an unexpected ping value");
    }

    return {
      "PostgreSQL",
      HealthStatus::Healthy,
      "The database accepted a SELECT 1 query."
    };
  } catch (const std::exception& error) {
    std::cerr << "PostgreSQL health check failed: " << error.what() << '\n';

    return {
      "PostgreSQL",
      HealthStatus::Unavailable,
      "The database is currently unavailable."
    };
  }
}

} // namespace kopds::system_health
