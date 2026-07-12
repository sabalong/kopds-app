#pragma once

#include <memory>

#include <Wt/WContainerWidget.h>

#include "contexts/membership/application/MemberService.h"
#include "contexts/system_health/application/HealthCheck.h"

namespace kopds::shell {

class DashboardPage final : public Wt::WContainerWidget {
public:
  DashboardPage(
    std::shared_ptr<const membership::MemberService> memberService,
    std::shared_ptr<const system_health::HealthCheck> postgresHealthCheck
  );
};

} // namespace kopds::shell
