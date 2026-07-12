#pragma once

#include <memory>
#include <string>

namespace Wt::Dbo {
class Session;
}

namespace kopds::shared {

class Database {
public:
  explicit Database(std::string connectionString);

  bool isConfigured() const;
  std::unique_ptr<Wt::Dbo::Session> openSession() const;

private:
  std::string connectionString_;
};

} // namespace kopds::shared
