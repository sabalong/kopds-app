#include "contexts/membership/application/MemberService.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace kopds::membership {
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

MemberService::MemberService(std::shared_ptr<const MemberRepository> repository)
  : repository_(std::move(repository))
{
}

MemberPageResult MemberService::list(const MemberListQuery& query) const
{
  return repository_->list(query);
}

MemberStats MemberService::stats() const
{
  return repository_->stats();
}

std::optional<Member> MemberService::find(const std::string& id) const
{
  return repository_->findById(id);
}

Member MemberService::create(const std::string& fullName) const
{
  return repository_->create(normalizedName(fullName));
}

Member MemberService::update(const std::string& id, const std::string& fullName) const
{
  const auto existing = repository_->findById(id);
  if (!existing || existing->deletedAt) {
    throw std::runtime_error("Member is not available for editing");
  }
  return repository_->update(id, normalizedName(fullName));
}

void MemberService::remove(const std::string& id) const
{
  repository_->setDeleted(id, true);
}

void MemberService::restore(const std::string& id) const
{
  repository_->setDeleted(id, false);
}

std::string MemberService::normalizedName(const std::string& fullName)
{
  const std::string result = trim(fullName);
  if (result.empty()) {
    throw std::invalid_argument("Full name is required");
  }
  return result;
}

} // namespace kopds::membership
