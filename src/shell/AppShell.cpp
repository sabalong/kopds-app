#include "shell/AppShell.h"

#include <memory>
#include <string>
#include <utility>

#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WLink.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "contexts/system_health/presentation/SystemStatusPage.h"
#include "contexts/membership/presentation/MembersPage.h"
#include "contexts/contribution_configuration/presentation/ContributionTypesPage.h"
#include "shell/DashboardPage.h"
#include "shell/NotFoundPage.h"

namespace kopds::shell {

AppShell::AppShell(
  std::shared_ptr<const system_health::HealthCheck> postgresHealthCheck,
  std::shared_ptr<const membership::MemberService> memberService,
  std::shared_ptr<const contribution_configuration::ContributionTypeService>
    contributionTypeService
)
  : postgresHealthCheck_(std::move(postgresHealthCheck)),
    memberService_(std::move(memberService)),
    contributionTypeService_(std::move(contributionTypeService))
{
  setStyleClass("app-shell");

  auto* sidebar = addNew<Wt::WContainerWidget>();
  sidebar->setStyleClass("app-sidebar");
  auto* brand = sidebar->addNew<Wt::WText>(
    "<div class=\"app-brand\"><span class=\"app-brand-mark\">K</span>"
    "<span>KOPDS</span></div>"
  );
  brand->setInline(false);

  auto* navigation = sidebar->addNew<Wt::WContainerWidget>();
  navigation->setStyleClass("app-navigation");

  dashboardLink_ = navigation->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath, "/dashboard"),
    "Dashboard"
  );
  dashboardLink_->setStyleClass("app-navigation-link");

  membersLink_ = navigation->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath, "/members"),
    "Members"
  );
  membersLink_->setStyleClass("app-navigation-link");

  auto* configurationLabel = navigation->addNew<Wt::WText>("Configuration");
  configurationLabel->setStyleClass("app-navigation-label");

  contributionTypesLink_ = navigation->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath,
              "/configuration/contribution-types"),
    "Contribution Types"
  );
  contributionTypesLink_->setStyleClass("app-navigation-link app-navigation-child");

  systemStatusLink_ = navigation->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath, "/system-status"),
    "System Status"
  );
  systemStatusLink_->setStyleClass("app-navigation-link");

  auto* main = addNew<Wt::WContainerWidget>();
  main->setStyleClass("app-main");

  auto* mobileHeader = main->addNew<Wt::WContainerWidget>();
  mobileHeader->setStyleClass("app-mobile-header");

  auto* menuButton = mobileHeader->addNew<Wt::WPushButton>("Menu");
  menuButton->setStyleClass("btn btn-outline-secondary app-menu-button");
  menuButton->setAttributeValue("aria-label", "Toggle navigation");
  menuButton->clicked().connect(this, &AppShell::toggleSidebar);

  mobileHeader->addNew<Wt::WText>("<strong>KOPDS</strong>");

  content_ = main->addNew<Wt::WContainerWidget>();
  content_->setStyleClass("app-content");

  auto* overlay = addNew<Wt::WPushButton>();
  overlay->setStyleClass("app-sidebar-overlay");
  overlay->setAttributeValue("aria-label", "Close navigation");
  overlay->clicked().connect([this] { setSidebarOpen(false); });

  Wt::WApplication* application = Wt::WApplication::instance();
  application->internalPathChanged().connect(this, &AppShell::handleRoute);

  if (application->internalPath().empty() || application->internalPath() == "/") {
    application->setInternalPath("/dashboard", true);
  } else {
    handleRoute();
  }
}

void AppShell::handleRoute()
{
  Wt::WApplication* application = Wt::WApplication::instance();
  const std::string path = application->internalPath();

  setSidebarOpen(false);
  content_->clear();
  updateActiveNavigation(path);

  if (path == "/dashboard") {
    application->setInternalPathValid(true);
    content_->addWidget(
      std::make_unique<DashboardPage>(memberService_, postgresHealthCheck_)
    );
  } else if (path.rfind("/members", 0) == 0) {
    application->setInternalPathValid(true);
    content_->addWidget(
      std::make_unique<membership::MembersPage>(memberService_, path)
    );
  } else if (path.rfind("/configuration/contribution-types", 0) == 0) {
    application->setInternalPathValid(true);
    content_->addWidget(
      std::make_unique<contribution_configuration::ContributionTypesPage>(
        contributionTypeService_, path
      )
    );
  } else if (path == "/system-status") {
    application->setInternalPathValid(true);
    content_->addWidget(
      std::make_unique<system_health::SystemStatusPage>(postgresHealthCheck_)
    );
  } else {
    application->setInternalPathValid(false);
    content_->addWidget(std::make_unique<NotFoundPage>(path));
  }
}

void AppShell::setSidebarOpen(bool open)
{
  sidebarOpen_ = open;
  toggleStyleClass("sidebar-open", sidebarOpen_);
}

void AppShell::toggleSidebar()
{
  setSidebarOpen(!sidebarOpen_);
}

void AppShell::updateActiveNavigation(const std::string& path)
{
  dashboardLink_->toggleStyleClass("active", path == "/dashboard");
  membersLink_->toggleStyleClass("active", path.rfind("/members", 0) == 0);
  contributionTypesLink_->toggleStyleClass(
    "active", path.rfind("/configuration/contribution-types", 0) == 0
  );
  systemStatusLink_->toggleStyleClass("active", path == "/system-status");
}

} // namespace kopds::shell
