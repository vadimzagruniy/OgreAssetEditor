#include "TransformGizmo.h"
#include "editor/render/RenderScene.h"
#include "editor/viewport/AxisColors.h"
#include "editor/viewport/ViewportFrame.h"
#include "editor/viewport/gizmo/GizmoView.h"

#include <OgreCamera.h>
#include <OgreMath.h>
#include <OgrePlane.h>
#include <OgreRay.h>
#include <imgui.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <utility>

namespace editor
{

namespace
{

constexpr float       AxisLengthScale      = 6.0f;
constexpr float       HoverDistanceScale   = 0.4f;
constexpr float       ArrowSizeScale       = 0.6f;
constexpr float       CenterSizeScale      = 0.35f;
constexpr float       PlaneStart           = 0.25f;
constexpr float       PlaneEnd             = 0.45f;
constexpr float       LineThickness        = 2.5f;
constexpr int         RingSegmentCount     = 64;
constexpr float       HiddenAxisAlignment  = 0.98f;
constexpr float       HiddenPlaneAlignment = 0.15f;
constexpr float       BackRingAlignment    = -0.1f;
constexpr float       MinimumScaleFactor   = 0.001f;
constexpr float       MinimumStartFraction = 0.01f;
constexpr float       MinimumClipDepth     = 1e-4f;
constexpr std::size_t AxisCount            = 3;

const ImU32 HighlightColor = IM_COL32(255, 210, 60, 255);
const ImU32 CenterColor    = IM_COL32(230, 230, 230, 255);

ImU32 WithAlpha(ImU32 _Color, float _Alpha)
{
  ImVec4 Color = ImGui::ColorConvertU32ToFloat4(_Color);
  Color.w *= _Alpha;
  return ImGui::ColorConvertFloat4ToU32(Color);
}

bool IsAxisHandle(EGizmoHandle _Handle)
{
  return _Handle >= EGizmoHandle::AxisX && _Handle <= EGizmoHandle::AxisZ;
}

bool IsPlaneHandle(EGizmoHandle _Handle)
{
  return _Handle >= EGizmoHandle::PlaneYZ && _Handle <= EGizmoHandle::PlaneXY;
}

std::size_t GetAxisIndex(EGizmoHandle _Handle)
{
  return static_cast<std::size_t>(_Handle) - static_cast<std::size_t>(EGizmoHandle::AxisX);
}

std::size_t GetPlaneIndex(EGizmoHandle _Handle)
{
  return static_cast<std::size_t>(_Handle) - static_cast<std::size_t>(EGizmoHandle::PlaneYZ);
}

EGizmoHandle MakeAxisHandle(std::size_t _AxisIndex)
{
  return static_cast<EGizmoHandle>(static_cast<std::size_t>(EGizmoHandle::AxisX) + _AxisIndex);
}

EGizmoHandle MakePlaneHandle(std::size_t _AxisIndex)
{
  return static_cast<EGizmoHandle>(static_cast<std::size_t>(EGizmoHandle::PlaneYZ) + _AxisIndex);
}

std::pair<std::size_t, std::size_t> GetOtherAxes(std::size_t _AxisIndex)
{
  return {(_AxisIndex + 1) % AxisCount, (_AxisIndex + 2) % AxisCount};
}

float GetAlignment(const TGizmoView & _View, std::size_t _AxisIndex)
{
  return std::abs(_View.Axes[_AxisIndex].dotProduct(_View.ToCamera));
}

bool IsAxisVisible(const TGizmoView & _View, std::size_t _AxisIndex)
{
  return GetAlignment(_View, _AxisIndex) < HiddenAxisAlignment;
}

bool IsPlaneVisible(const TGizmoView & _View, std::size_t _AxisIndex)
{
  return GetAlignment(_View, _AxisIndex) > HiddenPlaneAlignment;
}

std::optional<ImVec2> Project(const TGizmoView & _View, const Ogre::Vector3 & _Point)
{
  const Ogre::Vector4 Clip = _View.Projection * (_View.View * Ogre::Vector4(_Point.x, _Point.y, _Point.z, 1.0f));
  if (Clip.w <= MinimumClipDepth)
    return std::nullopt;

  const float NdcX = Clip.x / Clip.w;
  const float NdcY = Clip.y / Clip.w;

  return ImVec2(_View.Origin.x + (NdcX * 0.5f + 0.5f) * _View.Size.x, _View.Origin.y + (0.5f - NdcY * 0.5f) * _View.Size.y);
}

std::optional<ImVec2> ProjectAxisEnd(const TGizmoView & _View, std::size_t _AxisIndex)
{
  return Project(_View, _View.Position + _View.Axes[_AxisIndex] * _View.Length);
}

std::optional<std::array<ImVec2, 4>> ProjectPlane(const TGizmoView & _View, std::size_t _AxisIndex)
{
  const auto [FirstIndex, SecondIndex] = GetOtherAxes(_AxisIndex);

  const Ogre::Vector3 First  = _View.Axes[FirstIndex] * _View.Length;
  const Ogre::Vector3 Second = _View.Axes[SecondIndex] * _View.Length;

  const std::array<Ogre::Vector3, 4> Corners{
    _View.Position + First * PlaneStart + Second * PlaneStart,
    _View.Position + First * PlaneEnd + Second * PlaneStart,
    _View.Position + First * PlaneEnd + Second * PlaneEnd,
    _View.Position + First * PlaneStart + Second * PlaneEnd
  };

  std::array<ImVec2, 4> ScreenCorners;

  for (std::size_t CornerIndex = 0; CornerIndex < Corners.size(); ++CornerIndex)
  {
    const auto Corner = Project(_View, Corners[CornerIndex]);
    if (!Corner)
      return std::nullopt;

    ScreenCorners[CornerIndex] = *Corner;
  }

  return ScreenCorners;
}

Ogre::Vector3 GetRingDirection(const TGizmoView & _View, std::size_t _AxisIndex, int _SegmentIndex)
{
  const auto [FirstIndex, SecondIndex] = GetOtherAxes(_AxisIndex);

  const float Angle = Ogre::Math::TWO_PI * static_cast<float>(_SegmentIndex) / static_cast<float>(RingSegmentCount);

  return _View.Axes[FirstIndex] * std::cos(Angle) + _View.Axes[SecondIndex] * std::sin(Angle);
}

float DistanceToSegment(const ImVec2 & _Point, const ImVec2 & _Start, const ImVec2 & _End)
{
  const float SegmentX      = _End.x - _Start.x;
  const float SegmentY      = _End.y - _Start.y;
  const float LengthSquared = SegmentX * SegmentX + SegmentY * SegmentY;

  float Fraction = 0.0f;
  if (LengthSquared > 0.0f)
    Fraction = std::clamp(((_Point.x - _Start.x) * SegmentX + (_Point.y - _Start.y) * SegmentY) / LengthSquared, 0.0f, 1.0f);

  const float OffsetX = _Point.x - (_Start.x + SegmentX * Fraction);
  const float OffsetY = _Point.y - (_Start.y + SegmentY * Fraction);

  return std::sqrt(OffsetX * OffsetX + OffsetY * OffsetY);
}

bool IsInsideQuad(const ImVec2 & _Point, const std::array<ImVec2, 4> & _Corners)
{
  float Winding = 0.0f;

  for (std::size_t CornerIndex = 0; CornerIndex < _Corners.size(); ++CornerIndex)
  {
    const ImVec2 & Start = _Corners[CornerIndex];
    const ImVec2 & End   = _Corners[(CornerIndex + 1) % _Corners.size()];

    const float Cross = (End.x - Start.x) * (_Point.y - Start.y) - (End.y - Start.y) * (_Point.x - Start.x);
    if (Cross == 0.0f)
      continue;

    if (Winding == 0.0f)
      Winding = Cross;
    else if (Winding * Cross < 0.0f)
      return false;
  }

  return true;
}

float GetMouseAngle(const ImVec2 & _Center)
{
  const ImVec2 Mouse = ImGui::GetIO().MousePos;
  return std::atan2(Mouse.y - _Center.y, Mouse.x - _Center.x);
}

std::optional<float> ClosestAxisParameter(const Ogre::Ray & _Ray, const Ogre::Vector3 & _Origin, const Ogre::Vector3 & _Axis)
{
  const Ogre::Vector3 Offset      = _Origin - _Ray.getOrigin();
  const float         Alignment   = _Axis.dotProduct(_Ray.getDirection());
  const float         Denominator = 1.0f - Alignment * Alignment;

  if (Denominator < 1e-4f)
    return std::nullopt;

  return (Alignment * _Ray.getDirection().dotProduct(Offset) - _Axis.dotProduct(Offset)) / Denominator;
}

std::optional<Ogre::Vector3> IntersectPlane(const Ogre::Ray & _Ray, const Ogre::Vector3 & _Normal, const Ogre::Vector3 & _Point)
{
  const auto Hit = _Ray.intersects(Ogre::Plane(_Normal, _Point));
  if (!Hit.first)
    return std::nullopt;

  return _Ray.getPoint(Hit.second);
}

bool IsCenterHovered(const TGizmoView & _View, const ImVec2 & _Mouse)
{
  const float HalfSize = ImGui::GetFontSize() * CenterSizeScale;

  return std::abs(_Mouse.x - _View.ScreenPosition.x) <= HalfSize && std::abs(_Mouse.y - _View.ScreenPosition.y) <= HalfSize;
}

EGizmoHandle FindHoveredPlane(const TGizmoView & _View, const ImVec2 & _Mouse)
{
  for (std::size_t AxisIndex = 0; AxisIndex < AxisCount; ++AxisIndex)
  {
    if (!IsPlaneVisible(_View, AxisIndex))
      continue;

    const auto Corners = ProjectPlane(_View, AxisIndex);
    if (Corners && IsInsideQuad(_Mouse, *Corners))
      return MakePlaneHandle(AxisIndex);
  }

  return EGizmoHandle::None;
}

EGizmoHandle FindHoveredAxis(const TGizmoView & _View, const ImVec2 & _Mouse, float _Threshold)
{
  EGizmoHandle Hovered      = EGizmoHandle::None;
  float        BestDistance = _Threshold;

  for (std::size_t AxisIndex = 0; AxisIndex < AxisCount; ++AxisIndex)
  {
    if (!IsAxisVisible(_View, AxisIndex))
      continue;

    const auto End = ProjectAxisEnd(_View, AxisIndex);
    if (!End)
      continue;

    const float Distance = DistanceToSegment(_Mouse, _View.ScreenPosition, *End);
    if (Distance <= BestDistance)
    {
      BestDistance = Distance;
      Hovered      = MakeAxisHandle(AxisIndex);
    }
  }

  return Hovered;
}

EGizmoHandle FindHoveredRing(const TGizmoView & _View, const ImVec2 & _Mouse, float _Threshold)
{
  EGizmoHandle Hovered      = EGizmoHandle::None;
  float        BestDistance = _Threshold;

  for (std::size_t AxisIndex = 0; AxisIndex < AxisCount; ++AxisIndex)
  {
    std::optional<ImVec2> Previous;

    for (int SegmentIndex = 0; SegmentIndex <= RingSegmentCount; ++SegmentIndex)
    {
      const Ogre::Vector3 Direction = GetRingDirection(_View, AxisIndex, SegmentIndex);
      const auto          Point     = Project(_View, _View.Position + Direction * _View.Length);

      if (Previous && Point && Direction.dotProduct(_View.ToCamera) >= BackRingAlignment)
      {
        const float Distance = DistanceToSegment(_Mouse, *Previous, *Point);
        if (Distance <= BestDistance)
        {
          BestDistance = Distance;
          Hovered      = MakeAxisHandle(AxisIndex);
        }
      }

      Previous = Point;
    }
  }

  return Hovered;
}

void DrawArrowHead(ImDrawList & _DrawList, const ImVec2 & _Start, const ImVec2 & _End, float _Size, ImU32 _Color)
{
  const float DirectionX = _End.x - _Start.x;
  const float DirectionY = _End.y - _Start.y;
  const float Length     = std::sqrt(DirectionX * DirectionX + DirectionY * DirectionY);

  if (Length < 1.0f)
    return;

  const float UnitX = DirectionX / Length;
  const float UnitY = DirectionY / Length;
  const float Half  = _Size * 0.35f;

  const ImVec2 Tip(_End.x + UnitX * _Size, _End.y + UnitY * _Size);
  const ImVec2 Left(_End.x - UnitY * Half, _End.y + UnitX * Half);
  const ImVec2 Right(_End.x + UnitY * Half, _End.y - UnitX * Half);

  _DrawList.AddTriangleFilled(Tip, Left, Right, _Color);
}

void DrawSquare(ImDrawList & _DrawList, const ImVec2 & _Center, float _HalfSize, ImU32 _Color)
{
  _DrawList.AddRectFilled(ImVec2(_Center.x - _HalfSize, _Center.y - _HalfSize), ImVec2(_Center.x + _HalfSize, _Center.y + _HalfSize), _Color);
}

} // namespace

//
// Constructor
//

CTransformGizmo::CTransformGizmo(const CRenderScene & _Scene):
  m_Scene(_Scene),
  m_Mode(EGizmoMode::Translate),
  m_Space(EGizmoSpace::World),
  m_HoveredHandle(EGizmoHandle::None)
{
  // Empty
}

//
// Interface
//

void CTransformGizmo::SetMode(EGizmoMode _Mode)
{
  if (!m_Drag)
    m_Mode = _Mode;
}

EGizmoMode CTransformGizmo::GetMode() const noexcept
{
  return m_Mode;
}

void CTransformGizmo::SetSpace(EGizmoSpace _Space)
{
  if (!m_Drag)
    m_Space = _Space;
}

EGizmoSpace CTransformGizmo::GetSpace() const noexcept
{
  return m_Space;
}

bool CTransformGizmo::IsDragging() const noexcept
{
  return m_Drag.has_value();
}

bool CTransformGizmo::Draw(const TViewportFrame & _Frame, const Ogre::Affine3 & _Transform)
{
  const auto View        = BuildView(_Frame, _Transform);
  const bool WasDragging = m_Drag.has_value();

  if (WasDragging)
  {
    UpdateDrag(View, _Frame);
  }
  else
  {
    m_HoveredHandle = View && CanHover(_Frame) ? FindHoveredHandle(*View) : EGizmoHandle::None;

    if (m_HoveredHandle != EGizmoHandle::None && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      BeginDrag(*View, _Frame, m_HoveredHandle);
  }

  if (View)
  {
    auto & DrawList = *ImGui::GetWindowDrawList();

    DrawList.PushClipRect(_Frame.Origin, ImVec2(_Frame.Origin.x + _Frame.Size.x, _Frame.Origin.y + _Frame.Size.y), true);
    DrawHandles(DrawList, *View);
    DrawList.PopClipRect();
  }

  return WasDragging || m_HoveredHandle != EGizmoHandle::None;
}

void CTransformGizmo::OnDrag(std::function<void(const Ogre::Affine3 &)> _Callback)
{
  m_DragCallback = std::move(_Callback);
}

void CTransformGizmo::OnDragFinished(std::function<void()> _Callback)
{
  m_DragFinishedCallback = std::move(_Callback);
}

void CTransformGizmo::OnDragCanceled(std::function<void()> _Callback)
{
  m_DragCanceledCallback = std::move(_Callback);
}

//
// Implementation
//

std::optional<TGizmoView> CTransformGizmo::BuildView(const TViewportFrame & _Frame, const Ogre::Affine3 & _Transform) const
{
  const auto * Camera = m_Scene.GetCamera();

  TGizmoView View;
  View.Origin     = _Frame.Origin;
  View.Size       = _Frame.Size;
  View.Projection = Camera->getProjectionMatrix();
  View.View       = Camera->getViewMatrix();

  _Transform.decomposition(View.Position, View.Scale, View.Orientation);

  const Ogre::Vector3 ToCamera = Camera->getDerivedPosition() - View.Position;
  if (ToCamera.isZeroLength())
    return std::nullopt;

  View.ToCamera = ToCamera.normalisedCopy();

  const bool             IsLocal         = m_Space == EGizmoSpace::Local || m_Mode == EGizmoMode::Scale;
  const Ogre::Quaternion AxesOrientation = IsLocal ? View.Orientation : Ogre::Quaternion::IDENTITY;

  View.Axes = {AxesOrientation * Ogre::Vector3::UNIT_X, AxesOrientation * Ogre::Vector3::UNIT_Y, AxesOrientation * Ogre::Vector3::UNIT_Z};

  const auto ScreenPosition = Project(View, View.Position);
  const auto ScreenRight    = Project(View, View.Position + Camera->getDerivedRight());
  if (!ScreenPosition || !ScreenRight)
    return std::nullopt;

  const float PixelsPerUnit = std::hypot(ScreenRight->x - ScreenPosition->x, ScreenRight->y - ScreenPosition->y);
  if (PixelsPerUnit <= 0.0f)
    return std::nullopt;

  View.ScreenPosition = *ScreenPosition;
  View.PixelLength    = ImGui::GetFontSize() * AxisLengthScale;
  View.Length         = View.PixelLength / PixelsPerUnit;

  return View;
}

bool CTransformGizmo::CanHover(const TViewportFrame & _Frame) const
{
  const auto & Input = _Frame.Input;
  if (!Input.IsHovered || Input.IsAltDown || Input.IsRightDown || Input.IsMiddleDown)
    return false;

  return !Input.IsLeftDown || ImGui::IsMouseClicked(ImGuiMouseButton_Left);
}

EGizmoHandle CTransformGizmo::FindHoveredHandle(const TGizmoView & _View) const
{
  const ImVec2 Mouse     = ImGui::GetIO().MousePos;
  const float  Threshold = ImGui::GetFontSize() * HoverDistanceScale;

  switch (m_Mode)
  {
    case EGizmoMode::Translate:
    {
      const EGizmoHandle Plane = FindHoveredPlane(_View, Mouse);
      return Plane != EGizmoHandle::None ? Plane : FindHoveredAxis(_View, Mouse, Threshold);
    }

    case EGizmoMode::Rotate:
      return FindHoveredRing(_View, Mouse, Threshold);

    case EGizmoMode::Scale:
      return IsCenterHovered(_View, Mouse) ? EGizmoHandle::Center : FindHoveredAxis(_View, Mouse, Threshold);
  }

  return EGizmoHandle::None;
}

void CTransformGizmo::BeginDrag(const TGizmoView & _View, const TViewportFrame & _Frame, EGizmoHandle _Handle)
{
  TGizmoDrag Drag;
  Drag.Handle      = _Handle;
  Drag.Position    = _View.Position;
  Drag.Orientation = _View.Orientation;
  Drag.Scale       = _View.Scale;
  Drag.StartMouseX = ImGui::GetIO().MousePos.x;

  if (IsAxisHandle(_Handle))
    Drag.Axis = _View.Axes[GetAxisIndex(_Handle)];
  else if (IsPlaneHandle(_Handle))
    Drag.Axis = _View.Axes[GetPlaneIndex(_Handle)];

  const Ogre::Ray Ray = GetMouseRay(_Frame);

  if (m_Mode == EGizmoMode::Rotate)
  {
    Drag.PreviousAngle = GetMouseAngle(_View.ScreenPosition);
    Drag.AngleSign     = Drag.Axis.dotProduct(_View.ToCamera) > 0.0f ? -1.0f : 1.0f;
  }
  else if (IsPlaneHandle(_Handle))
  {
    const auto Hit = IntersectPlane(Ray, Drag.Axis, Drag.Position);
    if (!Hit)
      return;

    Drag.StartPoint = *Hit;
  }
  else if (IsAxisHandle(_Handle))
  {
    const auto Parameter = ClosestAxisParameter(Ray, Drag.Position, Drag.Axis);
    if (!Parameter || std::abs(*Parameter) < _View.Length * MinimumStartFraction)
      return;

    Drag.StartParameter = *Parameter;
  }

  m_Drag = Drag;
}

void CTransformGizmo::UpdateDrag(const std::optional<TGizmoView> & _View, const TViewportFrame & _Frame)
{
  auto & Drag = *m_Drag;

  if (!Drag.IsCanceled && (ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
  {
    Drag.IsCanceled = true;

    if (m_DragCanceledCallback)
      m_DragCanceledCallback();
  }

  if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
  {
    const bool IsCanceled = Drag.IsCanceled;
    m_Drag.reset();

    if (!IsCanceled && m_DragFinishedCallback)
      m_DragFinishedCallback();

    return;
  }

  if (Drag.IsCanceled || !_View)
    return;

  const auto Transform = CalcDragTransform(*_View, _Frame);
  if (Transform && m_DragCallback)
    m_DragCallback(*Transform);
}

std::optional<Ogre::Affine3> CTransformGizmo::CalcDragTransform(const TGizmoView & _View, const TViewportFrame & _Frame)
{
  switch (m_Mode)
  {
    case EGizmoMode::Translate: return CalcTranslation(_Frame);
    case EGizmoMode::Rotate:    return CalcRotation(_View);
    case EGizmoMode::Scale:     return CalcScale(_View, _Frame);
  }

  return std::nullopt;
}

std::optional<Ogre::Affine3> CTransformGizmo::CalcTranslation(const TViewportFrame & _Frame) const
{
  const auto &    Drag     = *m_Drag;
  const Ogre::Ray Ray      = GetMouseRay(_Frame);
  Ogre::Vector3   Position = Drag.Position;

  if (IsPlaneHandle(Drag.Handle))
  {
    const auto Hit = IntersectPlane(Ray, Drag.Axis, Drag.Position);
    if (!Hit)
      return std::nullopt;

    Position += *Hit - Drag.StartPoint;
  }
  else
  {
    const auto Parameter = ClosestAxisParameter(Ray, Drag.Position, Drag.Axis);
    if (!Parameter)
      return std::nullopt;

    Position += Drag.Axis * (*Parameter - Drag.StartParameter);
  }

  return Ogre::Affine3(Position, Drag.Orientation, Drag.Scale);
}

Ogre::Affine3 CTransformGizmo::CalcRotation(const TGizmoView & _View)
{
  auto & Drag = *m_Drag;

  const float MouseAngle = GetMouseAngle(_View.ScreenPosition);
  Drag.Angle        += std::remainder(MouseAngle - Drag.PreviousAngle, Ogre::Math::TWO_PI);
  Drag.PreviousAngle = MouseAngle;

  const Ogre::Quaternion Rotation(Ogre::Radian(Drag.Angle * Drag.AngleSign), Drag.Axis);

  return Ogre::Affine3(Drag.Position, Rotation * Drag.Orientation, Drag.Scale);
}

std::optional<Ogre::Affine3> CTransformGizmo::CalcScale(const TGizmoView & _View, const TViewportFrame & _Frame) const
{
  const auto &  Drag  = *m_Drag;
  Ogre::Vector3 Scale = Drag.Scale;

  if (Drag.Handle == EGizmoHandle::Center)
  {
    const float Offset = ImGui::GetIO().MousePos.x - Drag.StartMouseX;
    Scale *= std::max(1.0f + Offset / _View.PixelLength, MinimumScaleFactor);
  }
  else
  {
    const auto Parameter = ClosestAxisParameter(GetMouseRay(_Frame), Drag.Position, Drag.Axis);
    if (!Parameter)
      return std::nullopt;

    Scale[GetAxisIndex(Drag.Handle)] *= std::max(*Parameter / Drag.StartParameter, MinimumScaleFactor);
  }

  return Ogre::Affine3(Drag.Position, Drag.Orientation, Scale);
}

Ogre::Ray CTransformGizmo::GetMouseRay(const TViewportFrame & _Frame) const
{
  const ImVec2 Mouse = ImGui::GetIO().MousePos;

  return m_Scene.GetViewportRay((Mouse.x - _Frame.Origin.x) / _Frame.Size.x, (Mouse.y - _Frame.Origin.y) / _Frame.Size.y);
}

void CTransformGizmo::DrawHandles(ImDrawList & _DrawList, const TGizmoView & _View) const
{
  switch (m_Mode)
  {
    case EGizmoMode::Translate:
      DrawPlanes(_DrawList, _View);
      DrawAxes(_DrawList, _View);
      break;

    case EGizmoMode::Rotate:
      DrawRings(_DrawList, _View);
      break;

    case EGizmoMode::Scale:
      DrawAxes(_DrawList, _View);
      DrawCenter(_DrawList, _View);
      break;
  }
}

void CTransformGizmo::DrawAxes(ImDrawList & _DrawList, const TGizmoView & _View) const
{
  const float ArrowSize = ImGui::GetFontSize() * ArrowSizeScale;

  for (std::size_t AxisIndex = 0; AxisIndex < AxisCount; ++AxisIndex)
  {
    if (!IsAxisVisible(_View, AxisIndex))
      continue;

    const auto End = ProjectAxisEnd(_View, AxisIndex);
    if (!End)
      continue;

    const ImU32 Color = IsHighlighted(MakeAxisHandle(AxisIndex)) ? HighlightColor : GetAxisColor(AxisIndex);

    _DrawList.AddLine(_View.ScreenPosition, *End, Color, LineThickness);

    if (m_Mode == EGizmoMode::Scale)
      DrawSquare(_DrawList, *End, ArrowSize * 0.4f, Color);
    else
      DrawArrowHead(_DrawList, _View.ScreenPosition, *End, ArrowSize, Color);
  }
}

void CTransformGizmo::DrawPlanes(ImDrawList & _DrawList, const TGizmoView & _View) const
{
  for (std::size_t AxisIndex = 0; AxisIndex < AxisCount; ++AxisIndex)
  {
    if (!IsPlaneVisible(_View, AxisIndex))
      continue;

    const auto Corners = ProjectPlane(_View, AxisIndex);
    if (!Corners)
      continue;

    const ImU32 Color = IsHighlighted(MakePlaneHandle(AxisIndex)) ? HighlightColor : GetAxisColor(AxisIndex);
    const auto & [First, Second, Third, Fourth] = *Corners;

    _DrawList.AddQuadFilled(First, Second, Third, Fourth, WithAlpha(Color, 0.35f));
    _DrawList.AddQuad(First, Second, Third, Fourth, Color, 1.5f);
  }
}

void CTransformGizmo::DrawRings(ImDrawList & _DrawList, const TGizmoView & _View) const
{
  for (std::size_t AxisIndex = 0; AxisIndex < AxisCount; ++AxisIndex)
  {
    const ImU32 Color     = IsHighlighted(MakeAxisHandle(AxisIndex)) ? HighlightColor : GetAxisColor(AxisIndex);
    const ImU32 BackColor = WithAlpha(Color, 0.25f);

    std::optional<ImVec2> Previous;

    for (int SegmentIndex = 0; SegmentIndex <= RingSegmentCount; ++SegmentIndex)
    {
      const Ogre::Vector3 Direction = GetRingDirection(_View, AxisIndex, SegmentIndex);
      const auto          Point     = Project(_View, _View.Position + Direction * _View.Length);

      if (Previous && Point)
      {
        const bool IsFront = Direction.dotProduct(_View.ToCamera) >= BackRingAlignment;
        _DrawList.AddLine(*Previous, *Point, IsFront ? Color : BackColor, LineThickness);
      }

      Previous = Point;
    }
  }
}

void CTransformGizmo::DrawCenter(ImDrawList & _DrawList, const TGizmoView & _View) const
{
  const ImU32 Color = IsHighlighted(EGizmoHandle::Center) ? HighlightColor : CenterColor;

  DrawSquare(_DrawList, _View.ScreenPosition, ImGui::GetFontSize() * CenterSizeScale, Color);
}

bool CTransformGizmo::IsHighlighted(EGizmoHandle _Handle) const
{
  if (m_Drag)
    return !m_Drag->IsCanceled && m_Drag->Handle == _Handle;

  return m_HoveredHandle == _Handle;
}

} // namespace editor
