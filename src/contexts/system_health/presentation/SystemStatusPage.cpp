#include "contexts/system_health/presentation/SystemStatusPage.h"

#include <memory>
#include <string>
#include <utility>

#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "contexts/system_health/domain/ComponentHealth.h"

namespace kopds::system_health {
namespace {

std::string statusLabel(HealthStatus status)
{
  switch (status) {
  case HealthStatus::Healthy:
    return "Healthy";
  case HealthStatus::Unavailable:
    return "Unavailable";
  case HealthStatus::NotConfigured:
    return "Not configured";
  }

  return "Unknown";
}

std::string statusClass(HealthStatus status)
{
  switch (status) {
  case HealthStatus::Healthy:
    return "bg-success";
  case HealthStatus::Unavailable:
    return "bg-danger";
  case HealthStatus::NotConfigured:
    return "bg-secondary";
  }

  return "bg-secondary";
}

} // namespace

SystemStatusPage::SystemStatusPage(
  std::shared_ptr<const HealthCheck> postgresHealthCheck
)
  : getSystemStatus_(std::move(postgresHealthCheck))
{
  setStyleClass("page system-status-page");

  auto* heading = addNew<Wt::WContainerWidget>();
  heading->setStyleClass("page-heading");
  auto* headingText = heading->addNew<Wt::WText>(
    "<div><h1>System Status</h1>"
    "<p class=\"text-secondary mb-0\">Runtime health of application services.</p></div>"
  );
  headingText->setInline(false);

  auto* refreshButton = heading->addNew<Wt::WPushButton>("Refresh");
  refreshButton->setStyleClass("btn btn-outline-primary");
  refreshButton->clicked().connect(this, &SystemStatusPage::refresh);

  statusCards_ = addNew<Wt::WContainerWidget>();
  statusCards_->setStyleClass("status-grid");

  refresh();
}

void SystemStatusPage::refresh()
{
  statusCards_->clear();

  for (const ComponentHealth& component : getSystemStatus_.execute()) {
    auto* card = statusCards_->addNew<Wt::WContainerWidget>();
    card->setStyleClass("card status-card shadow-sm");

    auto* body = card->addNew<Wt::WContainerWidget>();
    body->setStyleClass("card-body");

    auto* titleRow = body->addNew<Wt::WContainerWidget>();
    titleRow->setStyleClass("status-card-title");

    auto* componentName = titleRow->addNew<Wt::WText>(component.component);
    componentName->setStyleClass("h5 mb-0");

    auto* badge = titleRow->addNew<Wt::WText>(statusLabel(component.status));
    badge->setStyleClass("badge " + statusClass(component.status));

    auto* message = body->addNew<Wt::WText>(component.message);
    message->setStyleClass("text-secondary status-message");
  }
}

} // namespace kopds::system_health
