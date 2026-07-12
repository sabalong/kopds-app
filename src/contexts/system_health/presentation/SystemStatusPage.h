#pragma once

#include <memory>

#include <Wt/WContainerWidget.h>

#include "contexts/system_health/application/GetSystemStatus.h"
#include "contexts/system_health/application/HealthCheck.h"

namespace kopds::system_health {

class SystemStatusPage final : public Wt::WContainerWidget {
public:
  explicit SystemStatusPage(std::shared_ptr<const HealthCheck> postgresHealthCheck);

private:
  GetSystemStatus getSystemStatus_;
  Wt::WContainerWidget* statusCards_{nullptr};

  void refresh();
};

} // namespace kopds::system_health
