#include "contexts/membership/infrastructure/PostgresMemberRepository.h"

#include <algorithm>
#include <tuple>
#include <utility>

#include <Wt/Dbo/Dbo.h>

namespace kopds::membership {
namespace {

using MemberRow = std::tuple<std::string, std::string, std::string,
                             std::string, std::string, std::string>;

const char* memberColumns =
  "id::text, member_number, full_name, "
  "to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD HH24:MI:SS') || ' UTC', "
  "to_char(updated_at AT TIME ZONE 'UTC', 'YYYY-MM-DD HH24:MI:SS') || ' UTC', "
  "coalesce(to_char(deleted_at AT TIME ZONE 'UTC', 'YYYY-MM-DD HH24:MI:SS') || ' UTC', '')";

Member toMember(const MemberRow& row)
{
  const std::string deletedAt = std::get<5>(row);
  return {
    std::get<0>(row), std::get<1>(row), std::get<2>(row),
    std::get<3>(row), std::get<4>(row),
    deletedAt.empty() ? std::nullopt : std::optional<std::string>(deletedAt)
  };
}

} // namespace

PostgresMemberRepository::PostgresMemberRepository(
  std::shared_ptr<const shared::Database> database
)
  : database_(std::move(database))
{
}

MemberPageResult PostgresMemberRepository::list(const MemberListQuery& query) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  const std::string search = query.search;
  const std::string pattern = "%" + search + "%";

  const int total = session->query<int>(
    "select count(*) from member "
    "where (? or deleted_at is null) "
    "and (? = '' or member_number ilike ? or full_name ilike ?)"
  ).bind(query.includeDeleted).bind(search).bind(pattern).bind(pattern).resultValue();

  const std::string sql = std::string("select ") + memberColumns +
    " from member where (? or deleted_at is null) "
    "and (? = '' or member_number ilike ? or full_name ilike ?) "
    "order by member_number limit ? offset ?";

  auto rows = session->query<MemberRow>(sql)
    .bind(query.includeDeleted).bind(search).bind(pattern).bind(pattern)
    .bind(query.pageSize).bind((std::max(1, query.page) - 1) * query.pageSize)
    .resultList();

  MemberPageResult result;
  result.total = total;
  for (const MemberRow& row : rows) {
    result.items.push_back(toMember(row));
  }
  transaction.commit();
  return result;
}

MemberStats PostgresMemberRepository::stats() const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  using StatsRow = std::tuple<long long, long long, long long>;
  const StatsRow row = session->query<StatsRow>(
    "select count(*), "
    "count(*) filter (where deleted_at is null), "
    "count(*) filter (where deleted_at is not null) from member"
  ).resultValue();
  transaction.commit();
  return {std::get<0>(row), std::get<1>(row), std::get<2>(row)};
}

std::optional<Member> PostgresMemberRepository::findById(const std::string& id) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  const std::string sql = std::string("select ") + memberColumns +
    " from member where id = ?::uuid";
  const auto rows = session->query<MemberRow>(sql).bind(id).resultList();
  for (const MemberRow& row : rows) {
    transaction.commit();
    return toMember(row);
  }
  transaction.commit();
  return std::nullopt;
}

Member PostgresMemberRepository::create(const std::string& fullName) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  const std::string id = session->query<std::string>(
    "select gen_random_uuid()::text"
  ).resultValue();
  session->execute(
    "insert into member (id, full_name) values (?::uuid, ?)"
  ).bind(id).bind(fullName).run();
  transaction.commit();
  return *findById(id);
}

Member PostgresMemberRepository::update(const std::string& id, const std::string& fullName) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  session->execute(
    "update member set full_name = ? "
    "where id = ?::uuid and deleted_at is null"
  ).bind(fullName).bind(id).run();
  transaction.commit();
  return *findById(id);
}

void PostgresMemberRepository::setDeleted(const std::string& id, bool deleted) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  session->execute(
    deleted
      ? "update member set deleted_at = current_timestamp where id = ?::uuid and deleted_at is null"
      : "update member set deleted_at = null where id = ?::uuid and deleted_at is not null"
  ).bind(id).run();
  transaction.commit();
}

} // namespace kopds::membership
