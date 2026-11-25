#pragma once

#include <functional>
#include <string>

namespace editor::ui
{

struct TModalButton
{
  std::string           Label;
  std::function<void()> Action;
  bool                  IsEnabled = true;
};

} // namespace editor::ui
