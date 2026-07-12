#pragma once

#include <optional>
#include <string>
#include <vector>

#include "contexts/membership/domain/Member.h"

namespace kopds::membership {

struct MemberListQuery {
  std::string search;
  bool includeDeleted{false};
  int page{1};
  int pageSize{25};
};

struct MemberPageResult {
  std::vector<Member> items;
  int total{0};
};

struct MemberStats {
  long long total{0};
  long long active{0};
  long long deleted{0};
};

class MemberRepository {
public:
  virtual ~MemberRepository() = default;
  virtual MemberPageResult list(const MemberListQuery& query) const = 0;
  virtual MemberStats stats() const = 0;
  virtual std::optional<Member> findById(const std::string& id) const = 0;
  virtual Member create(const std::string& fullName) const = 0;
  virtual Member update(const std::string& id, const std::string& fullName) const = 0;
  virtual void setDeleted(const std::string& id, bool deleted) const = 0;
};

} // namespace kopds::membership
