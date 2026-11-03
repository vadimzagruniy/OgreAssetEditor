#pragma once

#include "ViewportInput.h"

#include <imgui.h>

namespace editor
{

struct TViewportFrame
{
  ImVec2         Origin;
  ImVec2         Size;
  TViewportInput Input;
};

} // namespace editor
