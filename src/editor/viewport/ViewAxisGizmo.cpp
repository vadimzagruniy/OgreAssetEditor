#include "ViewAxisGizmo.h"
#include "AxisColors.h"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cstddef>

namespace editor
{

namespace
{

struct TAxisHandle
{
  Ogre::Vector3 Axis;
  ImVec2        Position;
  float         Depth;
  ImU32         Color;
  const char *  Label;
};

constexpr float GizmoRadiusScale  = 3.2f;
constexpr float HandleRadiusScale = 0.55f;
constexpr float MarginScale       = 0.8f;
constexpr float LineThickness     = 2.0f;

const ImU32 LabelColor      = IM_COL32(20, 21, 24, 255);
const ImU32 HoverHandleColor = IM_COL32(255, 255, 255, 255);
const ImU32 BackgroundColor  = IM_COL32(255, 255, 255, 18);

ImU32 FadeColor(ImU32 _Color, float _Alpha)
{
  ImVec4 Color = ImGui::ColorConvertU32ToFloat4(_Color);
  Color.w *= _Alpha;
  return ImGui::ColorConvertFloat4ToU32(Color);
}

std::array<TAxisHandle, 6> BuildHandles(const ImVec2 & _Center, float _Radius, const Ogre::Quaternion & _CameraOrientation)
{
  const std::array<Ogre::Vector3, 3> Axes{Ogre::Vector3::UNIT_X, Ogre::Vector3::UNIT_Y, Ogre::Vector3::UNIT_Z};
  const std::array<const char *, 3>  Labels{"X", "Y", "Z"};

  const Ogre::Quaternion ToView = _CameraOrientation.Inverse();

  std::array<TAxisHandle, 6> Handles;

  for (std::size_t AxisIndex = 0; AxisIndex < Axes.size(); ++AxisIndex)
  {
    for (const float Sign : {1.0f, -1.0f})
    {
      const Ogre::Vector3 Axis      = Axes[AxisIndex] * Sign;
      const Ogre::Vector3 ViewAxis  = ToView * Axis;
      const bool          IsPositive = Sign > 0.0f;

      Handles[AxisIndex * 2 + (IsPositive ? 0 : 1)] =
      {
        Axis,
        ImVec2(_Center.x + ViewAxis.x * _Radius, _Center.y - ViewAxis.y * _Radius),
        ViewAxis.z,
        GetAxisColor(AxisIndex),
        IsPositive ? Labels[AxisIndex] : nullptr
      };
    }
  }

  // Far handles are drawn first so the near ones cover them.
  std::sort(Handles.begin(), Handles.end(), [](const TAxisHandle & _Left, const TAxisHandle & _Right)
  {
    return _Left.Depth < _Right.Depth;
  });

  return Handles;
}

const TAxisHandle * FindHoveredHandle(const std::array<TAxisHandle, 6> & _Handles, float _HandleRadius)
{
  const ImVec2 Mouse = ImGui::GetIO().MousePos;

  // Handles are sorted far to near, so the last hit is the one on top.
  const TAxisHandle * Hovered = nullptr;

  for (const auto & Handle : _Handles)
  {
    const float OffsetX = Mouse.x - Handle.Position.x;
    const float OffsetY = Mouse.y - Handle.Position.y;

    if (OffsetX * OffsetX + OffsetY * OffsetY <= _HandleRadius * _HandleRadius)
      Hovered = &Handle;
  }

  return Hovered;
}

void DrawHandle(ImDrawList & _DrawList, const ImVec2 & _Center, const TAxisHandle & _Handle, float _HandleRadius, bool _IsHovered)
{
  const ImU32 FillColor = _IsHovered ? HoverHandleColor : _Handle.Color;

  if (!_Handle.Label)
  {
    _DrawList.AddCircleFilled(_Handle.Position, _HandleRadius * 0.75f, FadeColor(FillColor, 0.35f));
    _DrawList.AddCircle(_Handle.Position, _HandleRadius * 0.75f, FadeColor(_Handle.Color, 0.8f), 0, 1.5f);
    return;
  }

  _DrawList.AddLine(_Center, _Handle.Position, _Handle.Color, LineThickness);
  _DrawList.AddCircleFilled(_Handle.Position, _HandleRadius, FillColor);

  const ImVec2 LabelSize = ImGui::CalcTextSize(_Handle.Label);
  _DrawList.AddText(ImVec2(_Handle.Position.x - LabelSize.x * 0.5f, _Handle.Position.y - LabelSize.y * 0.5f), LabelColor, _Handle.Label);
}

} // namespace

std::optional<Ogre::Vector3> DrawViewAxisGizmo(const ImVec2 & _ViewportOrigin, const ImVec2 & _ViewportSize, const Ogre::Quaternion & _CameraOrientation)
{
  const float FontSize     = ImGui::GetFontSize();
  const float Radius       = FontSize * GizmoRadiusScale;
  const float HandleRadius = FontSize * HandleRadiusScale;
  const float Extent       = Radius + HandleRadius;
  const float Margin       = FontSize * MarginScale;

  const ImVec2 Center(_ViewportOrigin.x + _ViewportSize.x - Margin - Extent, _ViewportOrigin.y + Margin + Extent);

  if (_ViewportSize.x < Extent * 2.0f + Margin * 2.0f || _ViewportSize.y < Extent * 2.0f + Margin * 2.0f)
    return std::nullopt;

  ImGui::SetCursorScreenPos(ImVec2(Center.x - Extent, Center.y - Extent));
  ImGui::InvisibleButton("##ViewAxisGizmo", ImVec2(Extent * 2.0f, Extent * 2.0f));

  const bool  IsGizmoHovered = ImGui::IsItemHovered();
  const auto  Handles        = BuildHandles(Center, Radius, _CameraOrientation);
  const auto * HoveredHandle = IsGizmoHovered ? FindHoveredHandle(Handles, HandleRadius) : nullptr;

  auto & DrawList = *ImGui::GetWindowDrawList();

  if (IsGizmoHovered)
    DrawList.AddCircleFilled(Center, Extent, BackgroundColor);

  for (const auto & Handle : Handles)
    DrawHandle(DrawList, Center, Handle, HandleRadius, &Handle == HoveredHandle);

  if (HoveredHandle && ImGui::IsItemClicked(ImGuiMouseButton_Left))
    return HoveredHandle->Axis;

  return std::nullopt;
}

} // namespace editor
