#pragma once

#include <memory>

#include "contexts/contribution_configuration/application/ContributionTypeRepository.h"
#include "shared/infrastructure/Database.h"

namespace kopds::contribution_configuration {

class PostgresContributionTypeRepository final : public ContributionTypeRepository {
public:
  explicit PostgresContributionTypeRepository(
    std::shared_ptr<const shared::Database> database
  );

  ContributionTypePageResult list(const ContributionTypeListQuery& query) const override;
  std::optional<ContributionType> findById(const std::string& id) const override;
  ContributionType create(const ContributionTypeInput& input) const override;
  ContributionType update(const std::string& id, const ContributionTypeInput& input) const override;
  void setDeleted(const std::string& id, bool deleted) const override;

private:
  std::shared_ptr<const shared::Database> database_;
};

} // namespace kopds::contribution_configuration
