#pragma once

#include <memory>
#include <optional>
#include <string>

#include "contexts/contribution_configuration/application/ContributionTypeRepository.h"

namespace kopds::contribution_configuration {

class ContributionTypeService {
public:
  explicit ContributionTypeService(
    std::shared_ptr<const ContributionTypeRepository> repository
  );

  ContributionTypePageResult list(const ContributionTypeListQuery& query) const;
  std::optional<ContributionType> find(const std::string& id) const;
  ContributionType create(ContributionTypeInput input) const;
  ContributionType update(const std::string& id, ContributionTypeInput input) const;
  void remove(const std::string& id) const;
  void restore(const std::string& id) const;

private:
  std::shared_ptr<const ContributionTypeRepository> repository_;
  static ContributionTypeInput normalizeAndValidate(ContributionTypeInput input);
};

} // namespace kopds::contribution_configuration
