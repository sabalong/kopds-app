#include "contexts/contribution_configuration/infrastructure/PostgresContributionTypeRepository.h"

#include <algorithm>
#include <set>
#include <tuple>
#include <utility>

#include <Wt/Dbo/Dbo.h>

namespace kopds::contribution_configuration {
namespace {

using TypeRow = std::tuple<std::string, std::string, std::string, bool, bool,
                           bool, int, bool, std::string, std::string,
                           std::string, std::string>;
using TranslationRow = std::tuple<std::string, std::string, std::string,
                                  std::string, std::string, std::string,
                                  std::string>;

const char* typeColumns =
  "ct.id::text, ct.code, ct.category, ct.is_mandatory, "
  "ct.requires_active_loan, ct.is_refundable, ct.display_order, ct.active, "
  "coalesce((select i.name from contribution_type_i18n i "
  "where i.contribution_type_id = ct.id and i.locale = 'id-ID' and i.deleted_at is null limit 1), ct.code), "
  "to_char(ct.created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD HH24:MI:SS') || ' UTC', "
  "to_char(ct.updated_at AT TIME ZONE 'UTC', 'YYYY-MM-DD HH24:MI:SS') || ' UTC', "
  "coalesce(to_char(ct.deleted_at AT TIME ZONE 'UTC', 'YYYY-MM-DD HH24:MI:SS') || ' UTC', '')";

ContributionType toType(const TypeRow& row)
{
  const std::string deletedAt = std::get<11>(row);
  ContributionType result;
  result.id = std::get<0>(row);
  result.code = std::get<1>(row);
  result.category = std::get<2>(row);
  result.mandatory = std::get<3>(row);
  result.requiresActiveLoan = std::get<4>(row);
  result.refundable = std::get<5>(row);
  result.displayOrder = std::get<6>(row);
  result.active = std::get<7>(row);
  result.displayName = std::get<8>(row);
  result.createdAt = std::get<9>(row);
  result.updatedAt = std::get<10>(row);
  result.deletedAt = deletedAt.empty() ? std::nullopt : std::optional<std::string>(deletedAt);
  return result;
}

ContributionTranslation toTranslation(const TranslationRow& row)
{
  return {
    std::get<0>(row), std::get<1>(row), std::get<2>(row),
    std::get<3>(row), std::get<4>(row), std::get<5>(row), std::get<6>(row)
  };
}

void saveTranslations(
  Wt::Dbo::Session& session,
  const std::string& typeId,
  const std::vector<ContributionTranslation>& translations
)
{
  std::set<std::string> retainedLocales;
  for (const ContributionTranslation& translation : translations) {
    retainedLocales.insert(translation.locale);
    const int existingCount = session.query<int>(
      "select count(*) from contribution_type_i18n "
      "where contribution_type_id = ?::uuid and locale = ?"
    ).bind(typeId).bind(translation.locale).resultValue();

    if (existingCount > 0) {
      session.execute(
        "update contribution_type_i18n set name = ?, short_name = nullif(?, ''), "
        "description = nullif(?, ''), deleted_at = null "
        "where contribution_type_id = ?::uuid and locale = ?"
      ).bind(translation.name).bind(translation.shortName)
        .bind(translation.description).bind(typeId).bind(translation.locale).run();
    } else {
      session.execute(
        "insert into contribution_type_i18n "
        "(contribution_type_id, locale, name, short_name, description) "
        "values (?::uuid, ?, ?, nullif(?, ''), nullif(?, ''))"
      ).bind(typeId).bind(translation.locale).bind(translation.name)
        .bind(translation.shortName).bind(translation.description).run();
    }
  }

  auto existing = session.query<std::string>(
    "select locale from contribution_type_i18n "
    "where contribution_type_id = ?::uuid and deleted_at is null"
  ).bind(typeId).resultList();
  for (const std::string& locale : existing) {
    if (retainedLocales.count(locale) == 0) {
      session.execute(
        "update contribution_type_i18n set deleted_at = current_timestamp "
        "where contribution_type_id = ?::uuid and locale = ? and deleted_at is null"
      ).bind(typeId).bind(locale).run();
    }
  }
  session.execute(
    "update contribution_type set updated_at = current_timestamp where id = ?::uuid"
  ).bind(typeId).run();
}

std::vector<ContributionTranslation> loadTranslations(
  Wt::Dbo::Session& session,
  const std::string& typeId
)
{
  auto rows = session.query<TranslationRow>(
    "select id::text, locale, name, coalesce(short_name, ''), "
    "coalesce(description, ''), "
    "to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD HH24:MI:SS') || ' UTC', "
    "to_char(updated_at AT TIME ZONE 'UTC', 'YYYY-MM-DD HH24:MI:SS') || ' UTC' "
    "from contribution_type_i18n "
    "where contribution_type_id = ?::uuid and deleted_at is null order by locale"
  ).bind(typeId).resultList();
  std::vector<ContributionTranslation> result;
  for (const TranslationRow& row : rows) {
    result.push_back(toTranslation(row));
  }
  return result;
}

} // namespace

PostgresContributionTypeRepository::PostgresContributionTypeRepository(
  std::shared_ptr<const shared::Database> database
)
  : database_(std::move(database))
{
}

ContributionTypePageResult PostgresContributionTypeRepository::list(
  const ContributionTypeListQuery& query
) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  const std::string search = query.search;
  const std::string pattern = "%" + search + "%";
  const int total = session->query<int>(
    "select count(*) from contribution_type ct "
    "where (? or ct.deleted_at is null) and "
    "(? = '' or ct.code ilike ? or ct.category ilike ?)"
  ).bind(query.includeDeleted).bind(search).bind(pattern).bind(pattern).resultValue();

  const std::string sql = std::string("select ") + typeColumns +
    " from contribution_type ct where (? or ct.deleted_at is null) and "
    "(? = '' or ct.code ilike ? or ct.category ilike ?) "
    "order by ct.display_order, ct.code limit ? offset ?";
  auto rows = session->query<TypeRow>(sql)
    .bind(query.includeDeleted).bind(search).bind(pattern).bind(pattern)
    .bind(query.pageSize).bind((std::max(1, query.page) - 1) * query.pageSize)
    .resultList();
  ContributionTypePageResult result;
  result.total = total;
  for (const TypeRow& row : rows) {
    result.items.push_back(toType(row));
  }
  transaction.commit();
  return result;
}

std::optional<ContributionType> PostgresContributionTypeRepository::findById(
  const std::string& id
) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  const std::string sql = std::string("select ") + typeColumns +
    " from contribution_type ct where ct.id = ?::uuid";
  auto rows = session->query<TypeRow>(sql).bind(id).resultList();
  for (const TypeRow& row : rows) {
    ContributionType result = toType(row);
    result.translations = loadTranslations(*session, id);
    transaction.commit();
    return result;
  }
  transaction.commit();
  return std::nullopt;
}

ContributionType PostgresContributionTypeRepository::create(
  const ContributionTypeInput& input
) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  const std::string id = session->query<std::string>(
    "select gen_random_uuid()::text"
  ).resultValue();
  session->execute(
    "insert into contribution_type "
    "(id, code, category, is_mandatory, requires_active_loan, is_refundable, "
    "display_order, active) values (?::uuid, ?, ?, ?, ?, ?, ?, ?)"
  ).bind(id).bind(input.code).bind(input.category).bind(input.mandatory)
    .bind(input.requiresActiveLoan).bind(input.refundable)
    .bind(input.displayOrder).bind(input.active).run();
  saveTranslations(*session, id, input.translations);
  transaction.commit();
  return *findById(id);
}

ContributionType PostgresContributionTypeRepository::update(
  const std::string& id,
  const ContributionTypeInput& input
) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  session->execute(
    "update contribution_type set code = ?, category = ?, is_mandatory = ?, "
    "requires_active_loan = ?, is_refundable = ?, display_order = ?, active = ? "
    "where id = ?::uuid and deleted_at is null"
  ).bind(input.code).bind(input.category).bind(input.mandatory)
    .bind(input.requiresActiveLoan).bind(input.refundable)
    .bind(input.displayOrder).bind(input.active).bind(id).run();
  saveTranslations(*session, id, input.translations);
  transaction.commit();
  return *findById(id);
}

void PostgresContributionTypeRepository::setDeleted(
  const std::string& id,
  bool deleted
) const
{
  auto session = database_->openSession();
  Wt::Dbo::Transaction transaction(*session);
  session->execute(
    deleted
      ? "update contribution_type set deleted_at = current_timestamp where id = ?::uuid and deleted_at is null"
      : "update contribution_type set deleted_at = null where id = ?::uuid and deleted_at is not null"
  ).bind(id).run();
  transaction.commit();
}

} // namespace kopds::contribution_configuration
