#pragma once

#include <memory>

#include <Wt/WApplication.h>

#include "contexts/system_health/application/HealthCheck.h"
#include "contexts/membership/application/MemberService.h"
#include "contexts/contribution_configuration/application/ContributionTypeService.h"

namespace kopds::bootstrap {

class KopdsApplication final : public Wt::WApplication {
public:
  KopdsApplication(
    const Wt::WEnvironment& environment,
    std::shared_ptr<const system_health::HealthCheck> postgresHealthCheck,
    std::shared_ptr<const membership::MemberService> memberService,
    std::shared_ptr<const contribution_configuration::ContributionTypeService>
      contributionTypeService
  );
};

} // namespace kopds::bootstrap
