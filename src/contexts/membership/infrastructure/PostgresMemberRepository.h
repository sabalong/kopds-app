#pragma once

#include <memory>

#include "contexts/membership/application/MemberRepository.h"
#include "shared/infrastructure/Database.h"

namespace kopds::membership {

class PostgresMemberRepository final : public MemberRepository {
public:
  explicit PostgresMemberRepository(std::shared_ptr<const shared::Database> database);

  MemberPageResult list(const MemberListQuery& query) const override;
  MemberStats stats() const override;
  std::optional<Member> findById(const std::string& id) const override;
  Member create(const std::string& fullName) const override;
  Member update(const std::string& id, const std::string& fullName) const override;
  void setDeleted(const std::string& id, bool deleted) const override;

private:
  std::shared_ptr<const shared::Database> database_;
};

} // namespace kopds::membership
