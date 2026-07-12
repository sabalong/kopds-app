#pragma once

#include <optional>
#include <string>

namespace kopds::membership {

struct Member {
  std::string id;
  std::string memberNumber;
  std::string fullName;
  std::string createdAt;
  std::string updatedAt;
  std::optional<std::string> deletedAt;
};

} // namespace kopds::membership
