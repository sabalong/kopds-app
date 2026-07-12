#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WText.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>

namespace {

std::string pingPostgres()
{
  const char* databaseUrl = std::getenv("DATABASE_URL");
  if (databaseUrl == nullptr || *databaseUrl == '\0') {
    return "not configured";
  }

  auto connection = std::make_unique<Wt::Dbo::backend::Postgres>(databaseUrl);
  Wt::Dbo::Session session;
  session.setConnection(std::move(connection));

  Wt::Dbo::Transaction transaction(session);
  const int result = session.query<int>("select 1").resultValue();
  transaction.commit();

  if (result != 1) {
    throw std::runtime_error("PostgreSQL ping returned an unexpected result");
  }

  return "connected";
}

class HelloApplication final : public Wt::WApplication {
public:
  HelloApplication(const Wt::WEnvironment& environment,
                   const std::string& postgresStatus)
    : Wt::WApplication(environment)
  {
    setTitle("KOPDS App");
    root()->addWidget(std::make_unique<Wt::WText>(
      "<h1>KOPDS App</h1><p>PostgreSQL: " + postgresStatus + "</p>"
    ));
  }
};

} // namespace

int main(int argc, char** argv)
{
  try {
    const std::string postgresStatus = pingPostgres();
    std::cout << "PostgreSQL: " << postgresStatus << '\n';

    return Wt::WRun(argc, argv,
      [postgresStatus](const Wt::WEnvironment& environment) {
        return std::make_unique<HelloApplication>(environment, postgresStatus);
      });
  } catch (const std::exception& error) {
    std::cerr << "PostgreSQL ping failed: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
