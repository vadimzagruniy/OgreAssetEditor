#pragma once

#include "ui/dock_layout/DockSlot.h"

#include <string>

namespace editor::ui
{

struct TToolBoxDesc
{
  std::string Module;
  std::string Title;
  bool        Visible          = true;
  EDockSlot   DockSlot         = EDockSlot::None;
  int         DockOrder        = 0;
  bool        IsTitleBarHidden = false;
};

} // namespace editor::ui
