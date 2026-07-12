#include "shared/infrastructure/Database.h"

#include <memory>
#include <stdexcept>
#include <utility>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>

namespace kopds::shared {

Database::Database(std::string connectionString)
  : connectionString_(std::move(connectionString))
{
}

bool Database::isConfigured() const
{
  return !connectionString_.empty();
}

std::unique_ptr<Wt::Dbo::Session> Database::openSession() const
{
  if (!isConfigured()) {
    throw std::runtime_error("DATABASE_URL is not configured");
  }

  auto session = std::make_unique<Wt::Dbo::Session>();
  session->setConnection(
    std::make_unique<Wt::Dbo::backend::Postgres>(connectionString_)
  );
  return session;
}

} // namespace kopds::shared
