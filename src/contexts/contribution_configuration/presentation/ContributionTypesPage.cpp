#include "contexts/contribution_configuration/presentation/ContributionTypesPage.h"

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
#include <Wt/WSpinBox.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>

namespace kopds::contribution_configuration {
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

void addValue(Wt::WContainerWidget& parent, const std::string& label,
              const std::string& value)
{
  auto* group = parent.addNew<Wt::WContainerWidget>();
  group->setStyleClass("detail-field");
  group->addNew<Wt::WText>(label)->setStyleClass("detail-label");
  group->addNew<Wt::WText>(value, Wt::TextFormat::Plain)->setStyleClass("detail-value");
}

} // namespace

ContributionTypesPage::ContributionTypesPage(
  std::shared_ptr<const ContributionTypeService> service,
  std::string path
)
  : service_(std::move(service)), path_(std::move(path))
{
  setStyleClass("page contribution-types-page");
  const std::string base = "/configuration/contribution-types";
  if (path_ == base) {
    renderList();
  } else if (path_ == base + "/new") {
    renderForm("");
  } else {
    std::smatch match;
    const std::regex editPattern("^/configuration/contribution-types/([0-9a-fA-F-]+)/edit$");
    const std::regex detailPattern("^/configuration/contribution-types/([0-9a-fA-F-]+)$");
    if (std::regex_match(path_, match, editPattern)) {
      renderForm(match[1].str());
    } else if (std::regex_match(path_, match, detailPattern)) {
      renderDetail(match[1].str());
    } else {
      addHeading(*this, "Page not found", "The requested contribution-type route is invalid.");
    }
  }
}

void ContributionTypesPage::renderList()
{
  auto* heading = addNew<Wt::WContainerWidget>();
  heading->setStyleClass("page-heading");
  heading->addNew<Wt::WText>(
    "<div><h1>Contribution Types</h1><p class=\"text-secondary mb-0\">"
    "Configure contribution behavior and translations.</p></div>"
  )->setInline(false);
  auto* create = heading->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath,
              "/configuration/contribution-types/new"),
    "New contribution type"
  );
  create->setStyleClass("btn btn-primary");

  auto* filters = addNew<Wt::WContainerWidget>();
  filters->setStyleClass("crud-filters card card-body shadow-sm");
  search_ = filters->addNew<Wt::WLineEdit>();
  search_->setPlaceholderText("Search code or category");
  search_->setStyleClass("form-control");
  includeDeleted_ = filters->addNew<Wt::WCheckBox>("Include deleted");
  includeDeleted_->setStyleClass("crud-filter-check");
  auto* searchButton = filters->addNew<Wt::WPushButton>("Search");
  searchButton->setStyleClass("btn btn-outline-primary");
  error_ = addNew<Wt::WText>();
  error_->setStyleClass("alert alert-danger d-none");
  results_ = addNew<Wt::WContainerWidget>();
  searchButton->clicked().connect([this] { page_ = 1; loadList(); });
  search_->enterPressed().connect([this] { page_ = 1; loadList(); });
  includeDeleted_->changed().connect([this] { page_ = 1; loadList(); });
  loadList();
}

void ContributionTypesPage::loadList()
{
  results_->clear();
  try {
    ContributionTypeListQuery query;
    query.search = search_->text().toUTF8();
    query.includeDeleted = includeDeleted_->isChecked();
    query.page = page_;
    const ContributionTypePageResult result = service_->list(query);
    auto* wrap = results_->addNew<Wt::WContainerWidget>();
    wrap->setStyleClass("table-responsive crud-table-wrap");
    auto* table = wrap->addNew<Wt::WTable>();
    table->setStyleClass("table table-hover align-middle mb-0");
    const char* headers[] = {"Code", "Name", "Category", "Order", "Status", "Updated", ""};
    for (int column = 0; column < 7; ++column) {
      table->elementAt(0, column)->addNew<Wt::WText>(headers[column]);
    }
    int row = 1;
    for (const ContributionType& type : result.items) {
      table->elementAt(row, 0)->addNew<Wt::WText>(type.code);
      table->elementAt(row, 1)->addNew<Wt::WText>(type.displayName, Wt::TextFormat::Plain);
      table->elementAt(row, 2)->addNew<Wt::WText>(type.category, Wt::TextFormat::Plain);
      table->elementAt(row, 3)->addNew<Wt::WText>(std::to_string(type.displayOrder));
      const std::string state = type.deletedAt ? "Deleted" : (type.active ? "Active" : "Inactive");
      auto* badge = table->elementAt(row, 4)->addNew<Wt::WText>(state);
      badge->setStyleClass(type.deletedAt ? "badge bg-secondary" :
                           (type.active ? "badge bg-success" : "badge bg-warning text-dark"));
      table->elementAt(row, 5)->addNew<Wt::WText>(type.updatedAt);
      auto* view = table->elementAt(row, 6)->addNew<Wt::WAnchor>(
        Wt::WLink(Wt::LinkType::InternalPath,
                  "/configuration/contribution-types/" + type.id), "View"
      );
      view->setStyleClass("btn btn-sm btn-outline-primary");
      ++row;
    }
    if (result.items.empty()) {
      table->elementAt(1, 0)->setColumnSpan(7);
      table->elementAt(1, 0)->addNew<Wt::WText>("No contribution types found.");
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

void ContributionTypesPage::renderDetail(const std::string& id)
{
  try {
    const auto type = service_->find(id);
    if (!type) {
      addHeading(*this, "Contribution type not found", "The requested record does not exist.");
      return;
    }
    addHeading(*this, type->displayName, type->code);
    auto* card = addNew<Wt::WContainerWidget>();
    card->setStyleClass("card card-body shadow-sm detail-card");
    addValue(*card, "Code", type->code);
    addValue(*card, "Category", type->category);
    addValue(*card, "Mandatory", type->mandatory ? "Yes" : "No");
    addValue(*card, "Requires active loan", type->requiresActiveLoan ? "Yes" : "No");
    addValue(*card, "Refundable", type->refundable ? "Yes" : "No");
    addValue(*card, "Display order", std::to_string(type->displayOrder));
    addValue(*card, "Active", type->active ? "Yes" : "No");
    addValue(*card, "Created", type->createdAt);
    addValue(*card, "Last updated", type->updatedAt);
    addValue(*card, "Deleted", type->deletedAt.value_or("—"));

    auto* translationCard = addNew<Wt::WContainerWidget>();
    translationCard->setStyleClass("card card-body shadow-sm detail-card");
    translationCard->addNew<Wt::WText>("<h2 class=\"h5\">Translations</h2>");
    for (const ContributionTranslation& translation : type->translations) {
      addValue(*translationCard, translation.locale,
               translation.name + (translation.shortName.empty() ? "" : " (" + translation.shortName + ")"));
    }

    auto* actions = addNew<Wt::WContainerWidget>();
    actions->setStyleClass("crud-actions");
    if (!type->deletedAt) {
      auto* edit = actions->addNew<Wt::WAnchor>(
        Wt::WLink(Wt::LinkType::InternalPath,
                  "/configuration/contribution-types/" + id + "/edit"), "Edit"
      );
      edit->setStyleClass("btn btn-primary");
    }
    auto* lifecycle = actions->addNew<Wt::WPushButton>(type->deletedAt ? "Restore" : "Delete");
    lifecycle->setStyleClass(type->deletedAt ? "btn btn-success" : "btn btn-outline-danger");
    lifecycle->clicked().connect([this, id, deleted = bool(type->deletedAt)] {
      confirmation_.show(
        deleted ? "Restore contribution type" : "Delete contribution type",
        deleted ? "Restore this contribution type?" : "Soft-delete this contribution type?",
        deleted ? "Restore" : "Delete",
        !deleted,
        [this, id, deleted] {
          deleted ? service_->restore(id) : service_->remove(id);
          Wt::WApplication::instance()->setInternalPath(
            "/configuration/contribution-types", true
          );
        }
      );
    });
  } catch (const std::exception& error) {
    addHeading(*this, "Contribution types unavailable", error.what());
  }
}

void ContributionTypesPage::renderForm(const std::string& id)
{
  const bool editing = !id.empty();
  std::optional<ContributionType> existing;
  try {
    if (editing) {
      existing = service_->find(id);
      if (!existing || existing->deletedAt) {
        addHeading(*this, "Contribution type unavailable",
                   "Deleted or missing records cannot be edited.");
        return;
      }
    }
  } catch (const std::exception& error) {
    addHeading(*this, "Contribution types unavailable", error.what());
    return;
  }
  addHeading(*this, editing ? "Edit contribution type" : "New contribution type",
             "Configure behavior and localized labels.");
  auto* form = addNew<Wt::WContainerWidget>();
  form->setStyleClass("card card-body shadow-sm crud-form");
  auto addInput = [form](const std::string& label, const std::string& value) {
    auto* group = form->addNew<Wt::WContainerWidget>();
    group->setStyleClass("form-field");
    group->addNew<Wt::WText>(label)->setStyleClass("form-label");
    auto* input = group->addNew<Wt::WLineEdit>(value);
    input->setStyleClass("form-control");
    return input;
  };
  auto* code = addInput("Code", editing ? existing->code : "");
  auto* category = addInput("Category", editing ? existing->category : "");
  auto* displayOrder = form->addNew<Wt::WSpinBox>();
  displayOrder->setRange(0, 1000000);
  displayOrder->setValue(editing ? existing->displayOrder : 0);
  displayOrder->setStyleClass("form-control");
  auto* mandatory = form->addNew<Wt::WCheckBox>("Mandatory");
  auto* requiresLoan = form->addNew<Wt::WCheckBox>("Requires active loan");
  auto* refundable = form->addNew<Wt::WCheckBox>("Refundable");
  auto* active = form->addNew<Wt::WCheckBox>("Active");
  mandatory->setChecked(editing && existing->mandatory);
  requiresLoan->setChecked(editing && existing->requiresActiveLoan);
  refundable->setChecked(editing && existing->refundable);
  active->setChecked(!editing || existing->active);

  form->addNew<Wt::WText>("<h2 class=\"h5 mt-4\">Translations</h2>");
  translations_ = form->addNew<Wt::WContainerWidget>();
  translations_->setStyleClass("translation-list");
  if (editing) {
    for (const auto& translation : existing->translations) addTranslationRow(translation);
  } else {
    ContributionTranslation indonesian;
    indonesian.locale = "id-ID";
    addTranslationRow(indonesian);
  }
  auto* addTranslation = form->addNew<Wt::WPushButton>("Add translation");
  addTranslation->setStyleClass("btn btn-sm btn-outline-primary");
  addTranslation->clicked().connect([this] { addTranslationRow({}); });
  error_ = form->addNew<Wt::WText>();
  error_->setStyleClass("alert alert-danger d-none");
  auto* actions = form->addNew<Wt::WContainerWidget>();
  actions->setStyleClass("crud-actions");
  auto* save = actions->addNew<Wt::WPushButton>("Save");
  save->setStyleClass("btn btn-primary");
  auto* cancel = actions->addNew<Wt::WAnchor>(
    Wt::WLink(Wt::LinkType::InternalPath,
              editing ? "/configuration/contribution-types/" + id
                      : "/configuration/contribution-types"),
    "Cancel"
  );
  cancel->setStyleClass("btn btn-outline-secondary");
  save->clicked().connect([=] {
    try {
      ContributionTypeInput input = collectInput(
        code, category, mandatory, requiresLoan, refundable, displayOrder, active
      );
      const ContributionType saved = editing
        ? service_->update(id, std::move(input))
        : service_->create(std::move(input));
      Wt::WApplication::instance()->setInternalPath(
        "/configuration/contribution-types/" + saved.id, true
      );
    } catch (const std::exception& error) {
      showError(error.what());
    }
  });
}

void ContributionTypesPage::addTranslationRow(
  const ContributionTranslation& translation
)
{
  TranslationWidgets row;
  row.container = translations_->addNew<Wt::WContainerWidget>();
  row.container->setStyleClass("translation-row card card-body");
  auto addLine = [&row](const std::string& label, const std::string& value) {
    auto* group = row.container->addNew<Wt::WContainerWidget>();
    group->setStyleClass("form-field");
    group->addNew<Wt::WText>(label)->setStyleClass("form-label");
    auto* input = group->addNew<Wt::WLineEdit>(value);
    input->setStyleClass("form-control");
    return input;
  };
  row.locale = addLine("Locale", translation.locale);
  row.name = addLine("Name", translation.name);
  row.shortName = addLine("Short name", translation.shortName);
  auto* descriptionGroup = row.container->addNew<Wt::WContainerWidget>();
  descriptionGroup->setStyleClass("form-field");
  descriptionGroup->addNew<Wt::WText>("Description")->setStyleClass("form-label");
  row.description = descriptionGroup->addNew<Wt::WTextArea>(translation.description);
  row.description->setStyleClass("form-control");
  const std::size_t index = translationRows_.size();
  auto* remove = row.container->addNew<Wt::WPushButton>("Remove translation");
  remove->setStyleClass("btn btn-sm btn-outline-danger");
  remove->clicked().connect([this, index] {
    auto& selected = translationRows_.at(index);
    if (selected.locale->text().toUTF8() == "id-ID") {
      showError("The id-ID translation cannot be removed");
      return;
    }
    selected.included = false;
    selected.container->hide();
  });
  translationRows_.push_back(row);
}

ContributionTypeInput ContributionTypesPage::collectInput(
  Wt::WLineEdit* code, Wt::WLineEdit* category, Wt::WCheckBox* mandatory,
  Wt::WCheckBox* requiresLoan, Wt::WCheckBox* refundable,
  Wt::WSpinBox* displayOrder, Wt::WCheckBox* active
) const
{
  ContributionTypeInput input;
  input.code = code->text().toUTF8();
  input.category = category->text().toUTF8();
  input.mandatory = mandatory->isChecked();
  input.requiresActiveLoan = requiresLoan->isChecked();
  input.refundable = refundable->isChecked();
  input.displayOrder = displayOrder->value();
  input.active = active->isChecked();
  for (const TranslationWidgets& row : translationRows_) {
    if (!row.included) continue;
    ContributionTranslation translation;
    translation.locale = row.locale->text().toUTF8();
    translation.name = row.name->text().toUTF8();
    translation.shortName = row.shortName->text().toUTF8();
    translation.description = row.description->text().toUTF8();
    input.translations.push_back(std::move(translation));
  }
  return input;
}

void ContributionTypesPage::showError(const std::string& message)
{
  if (error_) {
    error_->setTextFormat(Wt::TextFormat::Plain);
    error_->setText(message);
    error_->removeStyleClass("d-none");
  }
}

} // namespace kopds::contribution_configuration
