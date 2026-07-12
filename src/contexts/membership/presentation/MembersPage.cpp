#include "contexts/membership/presentation/MembersPage.h"

#include <algorithm>
#include <exception>
#include <memory>
#include <regex>
#include <string>
#include <utility>

#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WCheckBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLink.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

namespace kopds::membership {
namespace {

void addHeading(Wt::WContainerWidget& page, const std::string& title,
                const std::string& subtitle)
{
  auto* text = page.addNew<Wt::WText>(
    "<div class=\"page-heading\"><div><h1>" + title + "</h1>"
    "<p class=\"text-secondary mb-0\">" + subtitle + "</p></div></div>"
  );
  text->setInline(false);
}

Wt::WText* addValue(Wt::WContainerWidget& parent, const std::string& label,
                    const std::string& value)
{
  auto* group = parent.addNew<Wt::WContainerWidget>();
  group->setStyleClass("detail-field");
  auto* labelText = group->addNew<Wt::WText>(label);
  labelText->setStyleClass("detail-label");
  auto* valueText = group->addNew<Wt::WText>(value, Wt::TextFormat::Plain);
  valueText->setStyleClass("detail-value");
  return valueText;
}

} // namespace

MembersPage::MembersPage(
  std::shared_ptr<const MemberService> service,
  std::string path
)
  : service_(std::move(service)), path_(std::move(path))
{
  setStyleClass("page members-page");
  if (path_ == "/members") {
    renderList();
  } else if (path_ == "/members/new") {
    renderForm("");
  } else {
    std::smatch match;
    const std::regex editPattern("^/members/([0-9a-fA-F-]+)/edit$");
    const std::regex detailPattern("^/members/([0-9a-fA-F-]+)$");
    if (std::regex_match(path_, match, editPattern)) {
      renderForm(match[1].str());
    } else if (std::regex_match(path_, match, detailPattern)) {
      renderDetail(match[1].str());
    } else {
      addHeading(*this, "Page not found", "The requested member route is invalid.");
    }
  }
}

void MembersPage::renderList()
{
  auto* heading = addNew<Wt::WContainerWidget>();
  heading->setStyleClass("page-heading");
  heading->addNew<Wt::WText>(
    "<div><h1>Members</h1><p class=\"text-secondary mb-0\">"
    "Manage cooperative members.</p></div>"
  )->setInline(false);
  auto* createLink = heading->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath, "/members/new"), "New member"
  );
  createLink->setStyleClass("btn btn-primary");

  auto* filters = addNew<Wt::WContainerWidget>();
  filters->setStyleClass("crud-filters card card-body shadow-sm");
  search_ = filters->addNew<Wt::WLineEdit>();
  search_->setPlaceholderText("Search member number or name");
  search_->setStyleClass("form-control");
  includeDeleted_ = filters->addNew<Wt::WCheckBox>("Include deleted");
  includeDeleted_->setStyleClass("crud-filter-check");
  auto* apply = filters->addNew<Wt::WPushButton>("Search");
  apply->setStyleClass("btn btn-outline-primary");
  apply->clicked().connect([this] { page_ = 1; loadList(); });
  search_->enterPressed().connect([this] { page_ = 1; loadList(); });
  includeDeleted_->changed().connect([this] { page_ = 1; loadList(); });

  error_ = addNew<Wt::WText>();
  error_->setStyleClass("alert alert-danger d-none");
  results_ = addNew<Wt::WContainerWidget>();
  loadList();
}

void MembersPage::loadList()
{
  results_->clear();
  try {
    MemberListQuery query;
    query.search = search_->text().toUTF8();
    query.includeDeleted = includeDeleted_->isChecked();
    query.page = page_;
    const MemberPageResult result = service_->list(query);

    auto* tableWrap = results_->addNew<Wt::WContainerWidget>();
    tableWrap->setStyleClass("table-responsive crud-table-wrap");
    auto* table = tableWrap->addNew<Wt::WTable>();
    table->setStyleClass("table table-hover align-middle mb-0");
    const char* headers[] = {"Member number", "Full name", "Last updated", "Status", ""};
    for (int column = 0; column < 5; ++column) {
      table->elementAt(0, column)->addNew<Wt::WText>(headers[column]);
    }
    int row = 1;
    for (const Member& member : result.items) {
      table->elementAt(row, 0)->addNew<Wt::WText>(member.memberNumber);
      table->elementAt(row, 1)->addNew<Wt::WText>(member.fullName, Wt::TextFormat::Plain);
      table->elementAt(row, 2)->addNew<Wt::WText>(member.updatedAt);
      auto* status = table->elementAt(row, 3)->addNew<Wt::WText>(
        member.deletedAt ? "Deleted" : "Active"
      );
      status->setStyleClass(member.deletedAt ? "badge bg-secondary" : "badge bg-success");
      auto* view = table->elementAt(row, 4)->addNew<Wt::WAnchor>(
        Wt::WLink(Wt::LinkType::InternalPath, "/members/" + member.id), "View"
      );
      view->setStyleClass("btn btn-sm btn-outline-primary");
      ++row;
    }
    if (result.items.empty()) {
      table->elementAt(1, 0)->setColumnSpan(5);
      table->elementAt(1, 0)->addNew<Wt::WText>("No members found.");
    }

    auto* pagination = results_->addNew<Wt::WContainerWidget>();
    pagination->setStyleClass("crud-pagination");
    auto* previous = pagination->addNew<Wt::WPushButton>("Previous");
    previous->setStyleClass("btn btn-sm btn-outline-secondary");
    previous->setDisabled(page_ <= 1);
    previous->clicked().connect([this] { --page_; loadList(); });
    pagination->addNew<Wt::WText>(
      "Page " + std::to_string(page_) + " · " + std::to_string(result.total) + " records"
    );
    auto* next = pagination->addNew<Wt::WPushButton>("Next");
    next->setStyleClass("btn btn-sm btn-outline-secondary");
    next->setDisabled(page_ * query.pageSize >= result.total);
    next->clicked().connect([this] { ++page_; loadList(); });
  } catch (const std::exception& error) {
    showError(error.what());
  }
}

void MembersPage::renderDetail(const std::string& id)
{
  try {
    const auto member = service_->find(id);
    if (!member) {
      addHeading(*this, "Member not found", "The requested member does not exist.");
      return;
    }
    addHeading(*this, member->fullName, member->memberNumber);
    auto* card = addNew<Wt::WContainerWidget>();
    card->setStyleClass("card card-body shadow-sm detail-card");
    addValue(*card, "Member number", member->memberNumber);
    addValue(*card, "Full name", member->fullName);
    addValue(*card, "Created", member->createdAt);
    addValue(*card, "Last updated", member->updatedAt);
    addValue(*card, "Deleted", member->deletedAt.value_or("—"));

    auto* actions = addNew<Wt::WContainerWidget>();
    actions->setStyleClass("crud-actions");
    if (!member->deletedAt) {
      auto* edit = actions->addNew<Wt::WAnchor>(
        Wt::WLink(Wt::LinkType::InternalPath, "/members/" + id + "/edit"), "Edit"
      );
      edit->setStyleClass("btn btn-primary");
    }
    auto* lifecycle = actions->addNew<Wt::WPushButton>(
      member->deletedAt ? "Restore" : "Delete"
    );
    lifecycle->setStyleClass(member->deletedAt ? "btn btn-success" : "btn btn-outline-danger");
    lifecycle->clicked().connect([this, id, deleted = bool(member->deletedAt)] {
      confirmation_.show(
        deleted ? "Restore member" : "Delete member",
        deleted ? "Restore this member?" : "Soft-delete this member?",
        deleted ? "Restore" : "Delete",
        !deleted,
        [this, id, deleted] {
          deleted ? service_->restore(id) : service_->remove(id);
          Wt::WApplication::instance()->setInternalPath("/members", true);
        }
      );
    });
  } catch (const std::exception& error) {
    addHeading(*this, "Members unavailable", error.what());
  }
}

void MembersPage::renderForm(const std::string& id)
{
  const bool editing = !id.empty();
  std::optional<Member> member;
  try {
    if (editing) {
      member = service_->find(id);
      if (!member || member->deletedAt) {
        addHeading(*this, "Member unavailable", "Deleted or missing members cannot be edited.");
        return;
      }
    }
  } catch (const std::exception& error) {
    addHeading(*this, "Members unavailable", error.what());
    return;
  }

  addHeading(*this, editing ? "Edit member" : "New member",
             editing ? member->memberNumber : "The member number is generated automatically.");
  auto* form = addNew<Wt::WContainerWidget>();
  form->setStyleClass("card card-body shadow-sm crud-form");
  auto* label = form->addNew<Wt::WText>("Full name");
  label->setStyleClass("form-label");
  auto* name = form->addNew<Wt::WLineEdit>(editing ? member->fullName : "");
  name->setStyleClass("form-control");
  error_ = form->addNew<Wt::WText>();
  error_->setStyleClass("alert alert-danger d-none");
  auto* actions = form->addNew<Wt::WContainerWidget>();
  actions->setStyleClass("crud-actions");
  auto* save = actions->addNew<Wt::WPushButton>("Save");
  save->setStyleClass("btn btn-primary");
  auto* cancel = actions->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath, editing ? "/members/" + id : "/members"),
    "Cancel"
  );
  cancel->setStyleClass("btn btn-outline-secondary");
  save->clicked().connect([this, id, editing, name] {
    try {
      const Member saved = editing
        ? service_->update(id, name->text().toUTF8())
        : service_->create(name->text().toUTF8());
      Wt::WApplication::instance()->setInternalPath("/members/" + saved.id, true);
    } catch (const std::exception& error) {
      showError(error.what());
    }
  });
}

void MembersPage::showError(const std::string& message)
{
  if (error_) {
    error_->setTextFormat(Wt::TextFormat::Plain);
    error_->setText(message);
    error_->removeStyleClass("d-none");
  }
}

} // namespace kopds::membership
