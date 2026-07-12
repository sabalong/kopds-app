#include "contexts/contribution_configuration/application/ContributionTypeService.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <set>
#include <stdexcept>
#include <utility>

namespace kopds::contribution_configuration {
namespace {

std::string trim(const std::string& value)
{
  const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) {
    return std::isspace(c);
  });
  const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) {
    return std::isspace(c);
  }).base();
  return first < last ? std::string(first, last) : std::string();
}

} // namespace

ContributionTypeService::ContributionTypeService(
  std::shared_ptr<const ContributionTypeRepository> repository
)
  : repository_(std::move(repository))
{
}

ContributionTypePageResult ContributionTypeService::list(
  const ContributionTypeListQuery& query
) const
{
  return repository_->list(query);
}

std::optional<ContributionType> ContributionTypeService::find(const std::string& id) const
{
  return repository_->findById(id);
}

ContributionType ContributionTypeService::create(ContributionTypeInput input) const
{
  return repository_->create(normalizeAndValidate(std::move(input)));
}

ContributionType ContributionTypeService::update(
  const std::string& id,
  ContributionTypeInput input
) const
{
  const auto existing = repository_->findById(id);
  if (!existing || existing->deletedAt) {
    throw std::runtime_error("Contribution type is not available for editing");
  }
  return repository_->update(id, normalizeAndValidate(std::move(input)));
}

void ContributionTypeService::remove(const std::string& id) const
{
  repository_->setDeleted(id, true);
}

void ContributionTypeService::restore(const std::string& id) const
{
  repository_->setDeleted(id, false);
}

ContributionTypeInput ContributionTypeService::normalizeAndValidate(
  ContributionTypeInput input
)
{
  input.code = trim(input.code);
  std::transform(input.code.begin(), input.code.end(), input.code.begin(), [](unsigned char c) {
    return static_cast<char>(std::toupper(c));
  });
  input.category = trim(input.category);

  if (!std::regex_match(input.code, std::regex("^[A-Z0-9_-]+$"))) {
    throw std::invalid_argument(
      "Code is required and may contain only A-Z, 0-9, underscore, and hyphen"
    );
  }
  if (input.category.empty()) {
    throw std::invalid_argument("Category is required");
  }
  if (input.displayOrder < 0) {
    throw std::invalid_argument("Display order cannot be negative");
  }

  std::set<std::string> locales;
  bool hasIndonesian = false;
  const std::regex localePattern("^[A-Za-z]{2,3}(-[A-Za-z0-9]{2,8})*$");
  for (ContributionTranslation& translation : input.translations) {
    translation.locale = trim(translation.locale);
    translation.name = trim(translation.name);
    translation.shortName = trim(translation.shortName);
    translation.description = trim(translation.description);
    if (!std::regex_match(translation.locale, localePattern)) {
      throw std::invalid_argument("Each translation requires a valid BCP-47 locale");
    }
    if (translation.name.empty()) {
      throw std::invalid_argument("Each translation requires a name");
    }
    if (!locales.insert(translation.locale).second) {
      throw std::invalid_argument("Translation locales must be unique");
    }
    hasIndonesian = hasIndonesian || translation.locale == "id-ID";
  }
  if (!hasIndonesian) {
    throw std::invalid_argument("An id-ID translation is required");
  }
  return input;
}

} // namespace kopds::contribution_configuration
