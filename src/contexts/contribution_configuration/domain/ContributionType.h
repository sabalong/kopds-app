#pragma once

#include <optional>
#include <string>
#include <vector>

namespace kopds::contribution_configuration {

struct ContributionTranslation {
  std::string id;
  std::string locale;
  std::string name;
  std::string shortName;
  std::string description;
  std::string createdAt;
  std::string updatedAt;
};

struct ContributionType {
  std::string id;
  std::string code;
  std::string category;
  bool mandatory{false};
  bool requiresActiveLoan{false};
  bool refundable{false};
  int displayOrder{0};
  bool active{true};
  std::string displayName;
  std::vector<ContributionTranslation> translations;
  std::string createdAt;
  std::string updatedAt;
  std::optional<std::string> deletedAt;
};

struct ContributionTypeInput {
  std::string code;
  std::string category;
  bool mandatory{false};
  bool requiresActiveLoan{false};
  bool refundable{false};
  int displayOrder{0};
  bool active{true};
  std::vector<ContributionTranslation> translations;
};

} // namespace kopds::contribution_configuration
