#include <cstdlib>
#include <memory>
#include <string>

#include <Wt/WApplication.h>

#include "bootstrap/KopdsApplication.h"
#include "contexts/membership/application/MemberService.h"
#include "contexts/membership/infrastructure/PostgresMemberRepository.h"
#include "contexts/contribution_configuration/application/ContributionTypeService.h"
#include "contexts/contribution_configuration/infrastructure/PostgresContributionTypeRepository.h"
#include "contexts/system_health/infrastructure/PostgresHealthCheck.h"
#include "shared/infrastructure/Database.h"

int main(int argc, char** argv)
{
  const char* configuredDatabaseUrl = std::getenv("DATABASE_URL");
  const std::string databaseUrl =
    configuredDatabaseUrl == nullptr ? "" : configuredDatabaseUrl;

  auto database = std::make_shared<kopds::shared::Database>(databaseUrl);
  auto postgresHealthCheck =
    std::make_shared<kopds::system_health::PostgresHealthCheck>(database);
  auto memberRepository =
    std::make_shared<kopds::membership::PostgresMemberRepository>(database);
  auto memberService =
    std::make_shared<kopds::membership::MemberService>(memberRepository);
  auto contributionTypeRepository = std::make_shared<
    kopds::contribution_configuration::PostgresContributionTypeRepository
  >(database);
  auto contributionTypeService = std::make_shared<
    kopds::contribution_configuration::ContributionTypeService
  >(contributionTypeRepository);

  return Wt::WRun(
    argc,
    argv,
    [postgresHealthCheck, memberService, contributionTypeService](
      const Wt::WEnvironment& environment
    ) {
      return std::make_unique<kopds::bootstrap::KopdsApplication>(
        environment,
        postgresHealthCheck,
        memberService,
        contributionTypeService
      );
    }
  );
}
