#pragma once

#include <functional>
#include <memory>
#include <string>

namespace Wt {
class WMessageBox;
}

namespace kopds::shared {

class ConfirmationDialog {
public:
  ConfirmationDialog();
  ~ConfirmationDialog();

  void show(
    const std::string& title,
    const std::string& message,
    const std::string& confirmLabel,
    bool destructive,
    std::function<void()> onConfirm
  );

private:
  std::unique_ptr<Wt::WMessageBox> dialog_;
  std::function<void()> onConfirm_;
};

} // namespace kopds::shared
