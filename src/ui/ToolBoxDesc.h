#pragma once

#include <string>

namespace editor::ui
{

struct TToolBoxDesc
{
  std::string Module;
  std::string Title;
  bool        Visible = true;
};

} // namespace editor::ui
