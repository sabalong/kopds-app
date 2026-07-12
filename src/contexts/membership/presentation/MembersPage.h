#pragma once

#include <memory>
#include <string>

#include <Wt/WContainerWidget.h>

#include "contexts/membership/application/MemberService.h"
#include "shared/presentation/ConfirmationDialog.h"

namespace Wt {
class WCheckBox;
class WLineEdit;
class WText;
}

namespace kopds::membership {

class MembersPage final : public Wt::WContainerWidget {
public:
  MembersPage(std::shared_ptr<const MemberService> service, std::string path);

private:
  std::shared_ptr<const MemberService> service_;
  std::string path_;
  Wt::WLineEdit* search_{nullptr};
  Wt::WCheckBox* includeDeleted_{nullptr};
  Wt::WContainerWidget* results_{nullptr};
  Wt::WText* error_{nullptr};
  int page_{1};
  shared::ConfirmationDialog confirmation_;

  void renderList();
  void loadList();
  void renderDetail(const std::string& id);
  void renderForm(const std::string& id);
  void showError(const std::string& message);
};

} // namespace kopds::membership
