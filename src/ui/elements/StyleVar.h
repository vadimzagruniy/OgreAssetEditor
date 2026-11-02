#pragma once

#include <imgui.h>

#include <variant>

namespace editor::ui
{

struct TStyleVar
{
  using TValue = std::variant<float, ImVec2>;

  ImGuiStyleVar Index;
  TValue        Value;
};

} // namespace editor::ui
