#include "shared/presentation/ConfirmationDialog.h"

#include <utility>

#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>

namespace kopds::shared {

ConfirmationDialog::ConfirmationDialog() = default;
ConfirmationDialog::~ConfirmationDialog() = default;

void ConfirmationDialog::show(
  const std::string& title,
  const std::string& message,
  const std::string& confirmLabel,
  bool destructive,
  std::function<void()> onConfirm
)
{
  onConfirm_ = std::move(onConfirm);
  dialog_ = std::make_unique<Wt::WMessageBox>(
    title,
    message,
    Wt::Icon::None,
    Wt::StandardButton::Yes | Wt::StandardButton::No
  );
  dialog_->setWidth("28rem");
  dialog_->setClosable(true);
  dialog_->rejectWhenEscapePressed(true);

  auto* confirmButton = dialog_->button(Wt::StandardButton::Yes);
  confirmButton->setText(confirmLabel);
  confirmButton->setStyleClass(destructive ? "btn btn-danger" : "btn btn-success");

  auto* cancelButton = dialog_->button(Wt::StandardButton::No);
  cancelButton->setText("Cancel");
  cancelButton->setStyleClass("btn btn-outline-secondary");
  dialog_->setDefaultButton(cancelButton);

  dialog_->buttonClicked().connect([this](Wt::StandardButton result) {
    std::function<void()> confirmedAction;
    if (result == Wt::StandardButton::Yes) {
      confirmedAction = std::move(onConfirm_);
    }
    dialog_.reset();
    onConfirm_ = {};
    if (confirmedAction) {
      confirmedAction();
    }
  });

  dialog_->show();
}

} // namespace kopds::shared
