#include "shell/DashboardPage.h"

#include <exception>
#include <string>
#include <utility>

#include <Wt/WAnchor.h>
#include <Wt/Chart/WPieChart.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WBrush.h>
#include <Wt/WColor.h>
#include <Wt/WLink.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WText.h>

#include "contexts/system_health/application/GetSystemStatus.h"
#include "contexts/system_health/domain/ComponentHealth.h"

namespace kopds::shell {
namespace {

std::string healthLabel(system_health::HealthStatus status)
{
  switch (status) {
  case system_health::HealthStatus::Healthy:
    return "Healthy";
  case system_health::HealthStatus::Unavailable:
    return "Unavailable";
  case system_health::HealthStatus::NotConfigured:
    return "Not configured";
  }
  return "Unknown";
}

std::string healthClass(system_health::HealthStatus status)
{
  switch (status) {
  case system_health::HealthStatus::Healthy:
    return "bg-success";
  case system_health::HealthStatus::Unavailable:
    return "bg-danger";
  case system_health::HealthStatus::NotConfigured:
    return "bg-secondary";
  }
  return "bg-secondary";
}

void addSummaryRow(
  Wt::WContainerWidget& parent,
  const std::string& label,
  long long value,
  const std::string& markerClass
)
{
  auto* row = parent.addNew<Wt::WContainerWidget>();
  row->setStyleClass("member-summary-row");
  auto* labelGroup = row->addNew<Wt::WContainerWidget>();
  labelGroup->setStyleClass("member-summary-label");
  labelGroup->addNew<Wt::WText>("")->setStyleClass(
    "member-summary-marker " + markerClass
  );
  labelGroup->addNew<Wt::WText>(label);
  row->addNew<Wt::WText>(std::to_string(value))->setStyleClass(
    "member-summary-value"
  );
}

} // namespace

DashboardPage::DashboardPage(
  std::shared_ptr<const membership::MemberService> memberService,
  std::shared_ptr<const system_health::HealthCheck> postgresHealthCheck
)
{
  setStyleClass("page dashboard-page");

  auto* heading = addNew<Wt::WText>(
    "<div class=\"page-heading\"><div><h1>Dashboard</h1>"
    "<p class=\"text-secondary mb-0\">Welcome to the KOPDS application.</p>"
    "</div></div>"
  );
  heading->setInline(false);

  auto* memberSection = addNew<Wt::WContainerWidget>();
  memberSection->setStyleClass("dashboard-section");
  auto* memberHeader = memberSection->addNew<Wt::WContainerWidget>();
  memberHeader->setStyleClass("dashboard-section-header");
  memberHeader->addNew<Wt::WText>("<h2 class=\"h5 mb-0\">Member Statistics</h2>");
  auto* memberLink = memberHeader->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath, "/members"),
    "Manage members"
  );
  memberLink->setStyleClass("btn btn-sm btn-outline-primary");

  auto* memberOverview = memberSection->addNew<Wt::WContainerWidget>();
  memberOverview->setStyleClass("dashboard-overview-grid");
  try {
    const membership::MemberStats stats = memberService->stats();
    auto* chartCard = memberOverview->addNew<Wt::WContainerWidget>();
    chartCard->setStyleClass("card shadow-sm dashboard-chart-card");
    auto* chartBody = chartCard->addNew<Wt::WContainerWidget>();
    chartBody->setStyleClass("card-body");
    chartBody->addNew<Wt::WText>("Member distribution")->setStyleClass(
      "dashboard-card-title"
    );

    if (stats.total > 0) {
      auto model = std::make_shared<Wt::WStandardItemModel>(2, 2);
      model->setData(0, 0, Wt::WString("Active"));
      model->setData(0, 1, stats.active);
      model->setData(1, 0, Wt::WString("Deleted"));
      model->setData(1, 1, stats.deleted);

      auto* chart = chartBody->addNew<Wt::Chart::WPieChart>();
      chart->setModel(model);
      chart->setLabelsColumn(0);
      chart->setDataColumn(1);
      chart->setDisplayLabels(
        Wt::Chart::LabelOption::Outside |
        Wt::Chart::LabelOption::TextLabel |
        Wt::Chart::LabelOption::TextPercentage
      );
      chart->setBrush(0, Wt::WBrush(Wt::WColor("#198754")));
      chart->setBrush(1, Wt::WBrush(Wt::WColor("#6c757d")));
      chart->setStartAngle(90);
      chart->resize(420, 260);
      chart->setStyleClass("member-chart-canvas");
    } else {
      auto* empty = chartBody->addNew<Wt::WText>(
        "No member data is available yet."
      );
      empty->setStyleClass("text-secondary dashboard-empty-chart");
    }

    auto* summaryCard = memberOverview->addNew<Wt::WContainerWidget>();
    summaryCard->setStyleClass("card shadow-sm member-summary-card");
    auto* summaryBody = summaryCard->addNew<Wt::WContainerWidget>();
    summaryBody->setStyleClass("card-body");
    summaryBody->addNew<Wt::WText>("Member counts")->setStyleClass(
      "dashboard-card-title"
    );
    addSummaryRow(*summaryBody, "Total", stats.total, "marker-primary");
    addSummaryRow(*summaryBody, "Active", stats.active, "marker-success");
    addSummaryRow(*summaryBody, "Deleted", stats.deleted, "marker-secondary");
  } catch (const std::exception&) {
    auto* unavailable = memberOverview->addNew<Wt::WText>(
      "Member statistics are unavailable because the database could not be queried."
    );
    unavailable->setStyleClass("alert alert-warning dashboard-wide-alert");
  }

  auto* systemSection = addNew<Wt::WContainerWidget>();
  systemSection->setStyleClass("dashboard-section");
  auto* systemHeader = systemSection->addNew<Wt::WContainerWidget>();
  systemHeader->setStyleClass("dashboard-section-header");
  systemHeader->addNew<Wt::WText>("<h2 class=\"h5 mb-0\">System Status</h2>");
  auto* statusLink = systemHeader->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath, "/system-status"),
    "View details"
  );
  statusLink->setStyleClass("btn btn-sm btn-outline-primary");

  auto* healthGrid = systemSection->addNew<Wt::WContainerWidget>();
  healthGrid->setStyleClass("status-grid dashboard-status-grid");
  system_health::GetSystemStatus getSystemStatus(std::move(postgresHealthCheck));
  for (const system_health::ComponentHealth& component : getSystemStatus.execute()) {
    auto* card = healthGrid->addNew<Wt::WContainerWidget>();
    card->setStyleClass("card status-card shadow-sm");
    auto* body = card->addNew<Wt::WContainerWidget>();
    body->setStyleClass("card-body");
    auto* title = body->addNew<Wt::WContainerWidget>();
    title->setStyleClass("status-card-title");
    title->addNew<Wt::WText>(component.component)->setStyleClass("h5 mb-0");
    auto* badge = title->addNew<Wt::WText>(healthLabel(component.status));
    badge->setStyleClass("badge " + healthClass(component.status));
    body->addNew<Wt::WText>(component.message)->setStyleClass(
      "text-secondary status-message"
    );
  }

}

} // namespace kopds::shell
