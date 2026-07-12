#pragma once

#include <memory>

#include <Wt/WContainerWidget.h>

#include "contexts/system_health/application/HealthCheck.h"
#include "contexts/membership/application/MemberService.h"
#include "contexts/contribution_configuration/application/ContributionTypeService.h"

namespace Wt {
class WAnchor;
}

namespace kopds::shell {

class AppShell final : public Wt::WContainerWidget {
public:
  explicit AppShell(
    std::shared_ptr<const system_health::HealthCheck> postgresHealthCheck,
    std::shared_ptr<const membership::MemberService> memberService,
    std::shared_ptr<const contribution_configuration::ContributionTypeService>
      contributionTypeService
  );

private:
  std::shared_ptr<const system_health::HealthCheck> postgresHealthCheck_;
  std::shared_ptr<const membership::MemberService> memberService_;
  std::shared_ptr<const contribution_configuration::ContributionTypeService>
    contributionTypeService_;
  Wt::WContainerWidget* content_{nullptr};
  Wt::WAnchor* dashboardLink_{nullptr};
  Wt::WAnchor* membersLink_{nullptr};
  Wt::WAnchor* contributionTypesLink_{nullptr};
  Wt::WAnchor* systemStatusLink_{nullptr};
  bool sidebarOpen_{false};

  void handleRoute();
  void setSidebarOpen(bool open);
  void toggleSidebar();
  void updateActiveNavigation(const std::string& path);
};

} // namespace kopds::shell
