#include "Viewport.h"
#include "ViewAxisGizmo.h"
#include "ViewportCamera.h"
#include "ViewportFrame.h"
#include "ViewportGrid.h"
#include "ViewportInput.h"
#include "ViewportNavigation.h"
#include "editor/render/RenderScene.h"
#include "ui/elements/Button.h"
#include "ui/elements/CheckBox.h"

#include <OgreCamera.h>
#include <OgreRay.h>
#include <OgreTexture.h>
#include <imgui.h>

#include <utility>

namespace editor
{

namespace
{

constexpr float NearClipFactor = 0.01f;
constexpr float FarClipFactor  = 1000.0f;

float ReadKeyAxis(ImGuiKey _Positive, ImGuiKey _Negative)
{
  return (ImGui::IsKeyDown(_Positive) ? 1.0f : 0.0f) - (ImGui::IsKeyDown(_Negative) ? 1.0f : 0.0f);
}

bool IsLeftDragged(const ImGuiIO & _InputState)
{
  const float Threshold = _InputState.MouseDragThreshold;
  return _InputState.MouseDragMaxDistanceSqr[ImGuiMouseButton_Left] > Threshold * Threshold;
}

TViewportInput ReadInput()
{
  const ImGuiIO & InputState = ImGui::GetIO();

  TViewportInput Input;
  Input.DeltaTime    = InputState.DeltaTime;
  Input.MouseDeltaX  = InputState.MouseDelta.x;
  Input.MouseDeltaY  = InputState.MouseDelta.y;
  Input.Wheel        = InputState.MouseWheel;
  Input.IsHovered    = ImGui::IsItemHovered();
  Input.IsCaptured   = ImGui::IsItemActive();
  Input.IsClicked    = ImGui::IsItemDeactivated() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !InputState.KeyAlt && !IsLeftDragged(InputState);
  Input.IsLeftDown   = ImGui::IsMouseDown(ImGuiMouseButton_Left);
  Input.IsRightDown  = ImGui::IsMouseDown(ImGuiMouseButton_Right);
  Input.IsMiddleDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
  Input.IsAltDown    = InputState.KeyAlt;
  Input.IsShiftDown  = InputState.KeyShift;

  // Movement keys belong to the viewport only while it holds the mouse, so they never reach other windows.
  if (Input.IsCaptured && Input.IsRightDown)
  {
    Input.MoveRight   = ReadKeyAxis(ImGuiKey_D, ImGuiKey_A);
    Input.MoveUp      = ReadKeyAxis(ImGuiKey_E, ImGuiKey_Q);
    Input.MoveForward = ReadKeyAxis(ImGuiKey_W, ImGuiKey_S);
  }

  return Input;
}

} // namespace

//
// Constructor/Destructor
//

CViewport::CViewport(CRenderScene & _Scene):
  m_Scene(_Scene),
  m_Camera(std::make_unique<CViewportCamera>(_Scene.GetCameraNode())),
  m_Navigation(std::make_unique<CViewportNavigation>()),
  m_Grid(std::make_unique<CViewportGrid>(_Scene.GetSceneManager())),
  m_Toolbar(ui::ELayoutDirection::Row),
  m_GridCheckBox(m_Toolbar.Add(ui::CCheckBox("Grid"))),
  m_FrameButton(m_Toolbar.Add(ui::CButton("Frame"))),
  m_ContentRevision(0),
  m_BoundsRevision(0)
{
  SetupToolbar();
}

CViewport::~CViewport() = default;

//
// Interface
//

void CViewport::Draw(const char * _Id)
{
  const ImVec2 Available = ImGui::GetContentRegionAvail();
  if (Available.x < 1.0f || Available.y < 1.0f)
    return;

  m_Scene.Resize(static_cast<unsigned int>(Available.x), static_cast<unsigned int>(Available.y));

  SyncContent();

  const ImVec2 Origin = ImGui::GetCursorScreenPos();

  // Overlays are submitted after the viewport and overlap it, so they take the mouse first.
  ImGui::SetNextItemAllowOverlap();
  ImGui::InvisibleButton(_Id, Available, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

  TViewportInput Input = ReadInput();

  DrawImage(Origin, Available);

  if (m_OverlayCallback && m_OverlayCallback({Origin, Available, Input}))
  {
    Input.IsClicked  = false;
    Input.IsLeftDown = false;
  }

  DrawToolbar(Origin);

  if (const auto AlignDirection = DrawViewAxisGizmo(Origin, Available, m_Camera->GetOrientation()))
    m_Camera->AlignTo(*AlignDirection);

  m_Navigation->Update(Input, *m_Camera);
  HandleShortcuts(Input);
  HandlePick(Input, Origin, Available);

  m_Camera->Update(Input.DeltaTime);
  m_Grid->Update(m_Camera->GetPosition(), m_Camera->GetPose().Distance);

  UpdateClipRange();
}

void CViewport::OnPick(std::function<void(const Ogre::Ray &)> _Callback)
{
  m_PickCallback = std::move(_Callback);
}

void CViewport::OnOverlay(std::function<bool(const TViewportFrame &)> _Callback)
{
  m_OverlayCallback = std::move(_Callback);
}

//
// Implementation
//

void CViewport::SetupToolbar()
{
  m_GridCheckBox.SetChecked(m_Grid->IsVisible()).SetVisible(m_Grid->IsAvailable()).OnToggled([this](bool _IsChecked)
  {
    m_Grid->SetVisible(_IsChecked);
  });

  m_FrameButton.SetTooltip("Frame the object (F)").OnClick([this]
  {
    m_Camera->Frame(m_Scene.GetContentBounds());
  });
}

void CViewport::DrawToolbar(const ImVec2 & _Origin)
{
  const float Margin = ImGui::GetStyle().WindowPadding.x;

  ImGui::SetCursorScreenPos(ImVec2(_Origin.x + Margin, _Origin.y + Margin));
  m_Toolbar.Draw();
}

void CViewport::SyncContent()
{
  const std::uint64_t ContentRevision = m_Scene.GetContentRevision();
  const std::uint64_t BoundsRevision  = m_Scene.GetBoundsRevision();

  if (ContentRevision != m_ContentRevision)
  {
    m_ContentRevision = ContentRevision;
    m_BoundsRevision  = BoundsRevision;
    m_Camera->Reset(m_Scene.GetContentBounds());
    return;
  }

  if (BoundsRevision == m_BoundsRevision)
    return;

  m_BoundsRevision = BoundsRevision;
  m_Camera->SetSceneBounds(m_Scene.GetContentBounds());
}

void CViewport::HandleShortcuts(const TViewportInput & _Input)
{
  if (!_Input.IsHovered || m_Navigation->GetMode() != EViewportNavigationMode::None || ImGui::GetIO().WantTextInput)
    return;

  if (ImGui::IsKeyPressed(ImGuiKey_F, false))
    m_Camera->Frame(m_Scene.GetContentBounds());
}

void CViewport::HandlePick(const TViewportInput & _Input, const ImVec2 & _Origin, const ImVec2 & _Size) const
{
  if (!_Input.IsClicked || !m_PickCallback)
    return;

  const ImVec2 MousePosition = ImGui::GetMousePos();
  const float  NormalizedX   = (MousePosition.x - _Origin.x) / _Size.x;
  const float  NormalizedY   = (MousePosition.y - _Origin.y) / _Size.y;

  m_PickCallback(m_Scene.GetViewportRay(NormalizedX, NormalizedY));
}

void CViewport::UpdateClipRange() const
{
  const float Distance = m_Camera->GetPose().Distance;
  auto *      Camera   = m_Scene.GetCamera();

  Camera->setNearClipDistance(Distance * NearClipFactor);
  Camera->setFarClipDistance(Distance * FarClipFactor);
}

void CViewport::DrawImage(const ImVec2 & _Origin, const ImVec2 & _Size) const
{
  const auto & Texture = m_Scene.GetTexture();
  if (!Texture)
    return;

  const auto TextureId = reinterpret_cast<ImTextureID>(Texture->getHandle());
  ImGui::GetWindowDrawList()->AddImage(TextureId, _Origin, ImVec2(_Origin.x + _Size.x, _Origin.y + _Size.y));
}

} // namespace editor
