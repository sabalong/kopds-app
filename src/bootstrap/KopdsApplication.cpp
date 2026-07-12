#include "bootstrap/KopdsApplication.h"

#include <memory>
#include <utility>

#include <Wt/WBootstrap5Theme.h>

#include "shell/AppShell.h"

namespace kopds::bootstrap {

KopdsApplication::KopdsApplication(
  const Wt::WEnvironment& environment,
  std::shared_ptr<const system_health::HealthCheck> postgresHealthCheck,
  std::shared_ptr<const membership::MemberService> memberService,
  std::shared_ptr<const contribution_configuration::ContributionTypeService>
    contributionTypeService
)
  : Wt::WApplication(environment)
{
  setTitle("KOPDS");
  setTheme(std::make_shared<Wt::WBootstrap5Theme>());
  useStyleSheet("/static/css/app.css");

  root()->setStyleClass("app-root");
  root()->addWidget(
    std::make_unique<shell::AppShell>(
      std::move(postgresHealthCheck),
      std::move(memberService),
      std::move(contributionTypeService)
    )
  );
}

} // namespace kopds::bootstrap
