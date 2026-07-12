#pragma once

#include <memory>
#include <optional>
#include <string>

#include "contexts/membership/application/MemberRepository.h"

namespace kopds::membership {

class MemberService {
public:
  explicit MemberService(std::shared_ptr<const MemberRepository> repository);

  MemberPageResult list(const MemberListQuery& query) const;
  MemberStats stats() const;
  std::optional<Member> find(const std::string& id) const;
  Member create(const std::string& fullName) const;
  Member update(const std::string& id, const std::string& fullName) const;
  void remove(const std::string& id) const;
  void restore(const std::string& id) const;

private:
  std::shared_ptr<const MemberRepository> repository_;
  static std::string normalizedName(const std::string& fullName);
};

} // namespace kopds::membership
