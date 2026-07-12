#pragma once

#include <optional>
#include <string>
#include <vector>

#include "contexts/contribution_configuration/domain/ContributionType.h"

namespace kopds::contribution_configuration {

struct ContributionTypeListQuery {
  std::string search;
  bool includeDeleted{false};
  int page{1};
  int pageSize{25};
};

struct ContributionTypePageResult {
  std::vector<ContributionType> items;
  int total{0};
};

class ContributionTypeRepository {
public:
  virtual ~ContributionTypeRepository() = default;
  virtual ContributionTypePageResult list(const ContributionTypeListQuery& query) const = 0;
  virtual std::optional<ContributionType> findById(const std::string& id) const = 0;
  virtual ContributionType create(const ContributionTypeInput& input) const = 0;
  virtual ContributionType update(const std::string& id, const ContributionTypeInput& input) const = 0;
  virtual void setDeleted(const std::string& id, bool deleted) const = 0;
};

} // namespace kopds::contribution_configuration
