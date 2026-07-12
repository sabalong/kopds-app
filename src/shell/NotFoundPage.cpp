#include "shell/NotFoundPage.h"

#include <Wt/WAnchor.h>
#include <Wt/WLink.h>
#include <Wt/WText.h>

namespace kopds::shell {

NotFoundPage::NotFoundPage(const std::string& path)
{
  setStyleClass("page not-found-page");
  addNew<Wt::WText>("<p class=\"not-found-code\">404</p>");
  auto* heading = addNew<Wt::WText>("<h1>Page not found</h1>");
  heading->setInline(false);

  auto* pathText = addNew<Wt::WText>(
    "No page is registered for " + path + ".",
    Wt::TextFormat::Plain
  );
  pathText->setStyleClass("text-secondary mb-4");

  auto* dashboardLink = addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath, "/dashboard"),
    "Return to dashboard"
  );
  dashboardLink->setStyleClass("btn btn-primary");
}

} // namespace kopds::shell
