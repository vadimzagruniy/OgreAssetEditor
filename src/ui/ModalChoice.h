#pragma once

#include <functional>
#include <string>

namespace editor::ui
{

struct TModalChoice
{
  std::string                               Label;
  std::function<bool(std::string & _Error)> Action;
};

} // namespace editor::ui
