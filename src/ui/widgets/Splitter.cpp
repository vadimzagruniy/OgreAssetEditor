#include "Splitter.h"

#include <imgui.h>

#include <algorithm>

namespace editor::ui
{

void HorizontalSplitter(const char * _Id, float & _Ratio, float _TotalHeight, float _MinRatio, float _MaxRatio)
{
  const float  Thickness = ImGui::GetStyle().ItemSpacing.y * 2.0f;
  const ImVec2 Position  = ImGui::GetCursorScreenPos();
  const float  Width     = ImGui::GetContentRegionAvail().x;

  ImGui::InvisibleButton(_Id, ImVec2(Width, Thickness));

  const bool IsHovered = ImGui::IsItemHovered();
  const bool IsActive  = ImGui::IsItemActive();

  if (IsHovered || IsActive)
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

  if (IsActive && _TotalHeight > 0.0f)
    _Ratio = std::clamp(_Ratio + ImGui::GetIO().MouseDelta.y / _TotalHeight, _MinRatio, _MaxRatio);

  const ImGuiCol ColorIndex = IsActive ? ImGuiCol_SeparatorActive : (IsHovered ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator);
  const float    LineY      = Position.y + Thickness * 0.5f;

  ImGui::GetWindowDrawList()->AddLine(ImVec2(Position.x, LineY), ImVec2(Position.x + Width, LineY), ImGui::GetColorU32(ColorIndex), IsHovered || IsActive ? 2.0f : 1.0f);
}

} // namespace editor::ui
