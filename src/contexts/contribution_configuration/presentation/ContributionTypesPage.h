#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Wt/WContainerWidget.h>

#include "contexts/contribution_configuration/application/ContributionTypeService.h"
#include "shared/presentation/ConfirmationDialog.h"

namespace Wt {
class WCheckBox;
class WLineEdit;
class WSpinBox;
class WText;
class WTextArea;
}

namespace kopds::contribution_configuration {

class ContributionTypesPage final : public Wt::WContainerWidget {
public:
  ContributionTypesPage(
    std::shared_ptr<const ContributionTypeService> service,
    std::string path
  );

private:
  struct TranslationWidgets {
    Wt::WContainerWidget* container{nullptr};
    Wt::WLineEdit* locale{nullptr};
    Wt::WLineEdit* name{nullptr};
    Wt::WLineEdit* shortName{nullptr};
    Wt::WTextArea* description{nullptr};
    bool included{true};
  };

  std::shared_ptr<const ContributionTypeService> service_;
  std::string path_;
  Wt::WLineEdit* search_{nullptr};
  Wt::WCheckBox* includeDeleted_{nullptr};
  Wt::WContainerWidget* results_{nullptr};
  Wt::WText* error_{nullptr};
  Wt::WContainerWidget* translations_{nullptr};
  std::vector<TranslationWidgets> translationRows_;
  int page_{1};
  shared::ConfirmationDialog confirmation_;

  void renderList();
  void loadList();
  void renderDetail(const std::string& id);
  void renderForm(const std::string& id);
  void addTranslationRow(const ContributionTranslation& translation);
  ContributionTypeInput collectInput(
    Wt::WLineEdit* code,
    Wt::WLineEdit* category,
    Wt::WCheckBox* mandatory,
    Wt::WCheckBox* requiresLoan,
    Wt::WCheckBox* refundable,
    Wt::WSpinBox* displayOrder,
    Wt::WCheckBox* active
  ) const;
  void showError(const std::string& message);
};

} // namespace kopds::contribution_configuration
