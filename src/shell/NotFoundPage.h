#pragma once

#include <string>

#include <Wt/WContainerWidget.h>

namespace kopds::shell {

class NotFoundPage final : public Wt::WContainerWidget {
public:
  explicit NotFoundPage(const std::string& path);
};

} // namespace kopds::shell
