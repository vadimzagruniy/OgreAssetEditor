#include "SceneViewportIcons.h"
#include "editor/render/RenderScene.h"
#include "editor/viewport/ViewportFrame.h"
#include "modules/scene/model/SceneGraph.h"
#include "modules/scene/model/SceneNode.h"
#include "modules/scene/model/SceneNodeTypes.h"

#include <OgreCamera.h>
#include <OgreMath.h>
#include <OgreVector.h>
#include <imgui.h>

#include <cmath>
#include <string>

namespace editor
{

namespace
{

constexpr float IconRadiusScale  = 0.55f;
constexpr float CoreRadiusScale  = 0.45f;
constexpr float RayStartScale    = 0.65f;
constexpr float LineThickness    = 2.0f;
constexpr float MinimumClipDepth = 0.0001f;
constexpr int   LightRayCount    = 8;

const ImU32 EmptyColour    = IM_COL32(200, 200, 200, 255);
const ImU32 LightColour    = IM_COL32(255, 214, 102, 255);
const ImU32 HoveredColour  = IM_COL32(255, 255, 255, 255);
const ImU32 SelectedColour = IM_COL32(255, 150, 40, 255);

float GetIconRadius()
{
  return ImGui::GetFontSize() * IconRadiusScale;
}

ESceneNodeIcon GetIcon(const CSceneGraph & _Graph, const std::string & _Name)
{
  return _Graph.GetNodeTypes().GetIcon(std::string(_Graph.GetNode(_Name).GetTypeId()));
}

bool CanHover(const TViewportFrame & _Frame)
{
  const auto & Input = _Frame.Input;
  return Input.IsHovered && !Input.IsRightDown && !Input.IsMiddleDown && !Input.IsAltDown;
}

ImU32 GetIconColour(ESceneNodeIcon _Icon, bool _IsSelected, bool _IsHovered)
{
  if (_IsSelected)
    return SelectedColour;

  if (_IsHovered)
    return HoveredColour;

  return _Icon == ESceneNodeIcon::Light ? LightColour : EmptyColour;
}

void DrawEmptyIcon(ImDrawList & _DrawList, const ImVec2 & _Center, float _Radius, ImU32 _Colour)
{
  _DrawList.AddLine(ImVec2(_Center.x - _Radius, _Center.y), ImVec2(_Center.x + _Radius, _Center.y), _Colour, LineThickness);
  _DrawList.AddLine(ImVec2(_Center.x, _Center.y - _Radius), ImVec2(_Center.x, _Center.y + _Radius), _Colour, LineThickness);
  _DrawList.AddCircle(_Center, _Radius * CoreRadiusScale, _Colour, 0, LineThickness);
}

void DrawLightIcon(ImDrawList & _DrawList, const ImVec2 & _Center, float _Radius, ImU32 _Colour)
{
  _DrawList.AddCircleFilled(_Center, _Radius * CoreRadiusScale, _Colour);

  for (int RayIndex = 0; RayIndex < LightRayCount; ++RayIndex)
  {
    const float  Angle     = Ogre::Math::TWO_PI * static_cast<float>(RayIndex) / static_cast<float>(LightRayCount);
    const ImVec2 Direction(std::cos(Angle), std::sin(Angle));

    const ImVec2 RayStart(_Center.x + Direction.x * _Radius * RayStartScale, _Center.y + Direction.y * _Radius * RayStartScale);
    const ImVec2 RayEnd(_Center.x + Direction.x * _Radius, _Center.y + Direction.y * _Radius);

    _DrawList.AddLine(RayStart, RayEnd, _Colour, LineThickness);
  }
}

void DrawIcon(ImDrawList & _DrawList, ESceneNodeIcon _Icon, const ImVec2 & _Center, float _Radius, ImU32 _Colour)
{
  switch (_Icon)
  {
    case ESceneNodeIcon::Empty:
      DrawEmptyIcon(_DrawList, _Center, _Radius, _Colour);
      break;

    case ESceneNodeIcon::Light:
      DrawLightIcon(_DrawList, _Center, _Radius, _Colour);
      break;

    case ESceneNodeIcon::None:
      break;
  }
}

} // namespace

//
// Constructor
//

CSceneViewportIcons::CSceneViewportIcons(const CRenderScene & _RenderScene):
  m_RenderScene(_RenderScene)
{
  // Empty
}

//
// Interface
//

std::string CSceneViewportIcons::Draw(const TViewportFrame & _Frame, const CSceneGraph & _Graph, const std::string & _Selection) const
{
  const std::string Hovered = CanHover(_Frame) ? FindHovered(_Frame, _Graph) : std::string();
  const float       Radius  = GetIconRadius();

  auto & DrawList = *ImGui::GetWindowDrawList();
  DrawList.PushClipRect(_Frame.Origin, ImVec2(_Frame.Origin.x + _Frame.Size.x, _Frame.Origin.y + _Frame.Size.y), true);

  for (const auto & Name : _Graph.GetNodeNames())
  {
    const ESceneNodeIcon Icon = GetIcon(_Graph, Name);
    if (Icon == ESceneNodeIcon::None)
      continue;

    const auto Center = FindScreenPosition(_Frame, _Graph, Name);
    if (!Center)
      continue;

    DrawIcon(DrawList, Icon, *Center, Radius, GetIconColour(Icon, Name == _Selection, Name == Hovered));
  }

  DrawList.PopClipRect();

  return Hovered;
}

//
// Implementation
//

std::string CSceneViewportIcons::FindHovered(const TViewportFrame & _Frame, const CSceneGraph & _Graph) const
{
  const ImVec2 Mouse = ImGui::GetMousePos();

  std::string Hovered;
  float       BestDistance = GetIconRadius();

  for (const auto & Name : _Graph.GetNodeNames())
  {
    if (GetIcon(_Graph, Name) == ESceneNodeIcon::None)
      continue;

    const auto Center = FindScreenPosition(_Frame, _Graph, Name);
    if (!Center)
      continue;

    const float Distance = std::hypot(Mouse.x - Center->x, Mouse.y - Center->y);
    if (Distance > BestDistance)
      continue;

    Hovered      = Name;
    BestDistance = Distance;
  }

  return Hovered;
}

std::optional<ImVec2> CSceneViewportIcons::FindScreenPosition(const TViewportFrame & _Frame, const CSceneGraph & _Graph, const std::string & _Name) const
{
  return Project(_Frame, _Graph.GetWorldTransform(_Name).getTrans());
}

std::optional<ImVec2> CSceneViewportIcons::Project(const TViewportFrame & _Frame, const Ogre::Vector3 & _Point) const
{
  const auto *        Camera = m_RenderScene.GetCamera();
  const Ogre::Vector4 Clip   = Camera->getProjectionMatrix() * (Camera->getViewMatrix() * Ogre::Vector4(_Point.x, _Point.y, _Point.z, 1.0f));

  if (Clip.w <= MinimumClipDepth)
    return std::nullopt;

  const float NdcX = Clip.x / Clip.w;
  const float NdcY = Clip.y / Clip.w;

  return ImVec2(_Frame.Origin.x + (NdcX * 0.5f + 0.5f) * _Frame.Size.x, _Frame.Origin.y + (0.5f - NdcY * 0.5f) * _Frame.Size.y);
}

} // namespace editor
