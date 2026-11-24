#include "SceneWorkspace.h"
#include "SceneChange.h"
#include "SceneDocument.h"
#include "SceneSelection.h"
#include "SceneTemplatePayload.h"
#include "editor/assets/AssetPayload.h"
#include "editor/render/RenderScene.h"
#include "editor/viewport/gizmo/TransformGizmo.h"
#include "editor/viewport/Viewport.h"
#include "editor/viewport/ViewportFrame.h"
#include "inspector/SceneNodePropertySource.h"
#include "model/nodes/LightNode.h"
#include "presenter/SceneEditorController.h"
#include "render/SceneSync.h"
#include "ui/DragPayload.h"
#include "ui/DropPayload.h"
#include "ui/elements/ComboBox.h"
#include "views/SceneObjectsWindow.h"
#include "views/SceneViewportIcons.h"
#include "views/SceneOutlinerWindow.h"

#include <OgreCamera.h>
#include <OgrePlane.h>
#include <OgreRay.h>
#include <imgui.h>

#include <array>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace editor
{

namespace
{

constexpr std::array<const char *, 3> ModeLabels{"Move", "Rotate", "Scale"};
constexpr std::array<const char *, 2> SpaceLabels{"World", "Local"};

constexpr float ComboWidthScale = 6.0f;

bool IsMouseInFrame(const TViewportFrame & _Frame)
{
  return ImGui::IsMouseHoveringRect(_Frame.Origin, ImVec2(_Frame.Origin.x + _Frame.Size.x, _Frame.Origin.y + _Frame.Size.y));
}

} // namespace

//
// Constructor/Destructor
//

CSceneWorkspace::CSceneWorkspace(std::shared_ptr<CSceneDocument> _Document, std::shared_ptr<CSceneEditorController> _Controller, std::string _ResourceGroup, std::shared_ptr<const CSceneNodeBindings> _NodeBindings, std::shared_ptr<core::CLog> _Log):
  CEditorWorkspace(_Document),
  m_Document(std::move(_Document)),
  m_Selection(std::make_shared<CSceneSelection>()),
  m_Controller(std::move(_Controller)),
  m_NodeProperties(std::make_shared<CSceneNodePropertySource>(m_Document, m_Selection)),
  m_RenderScene(std::make_unique<CRenderScene>()),
  m_Sync(std::make_unique<CSceneSync>(*m_RenderScene, std::move(_ResourceGroup), std::move(_NodeBindings), std::move(_Log))),
  m_Viewport(std::make_unique<CViewport>(*m_RenderScene)),
  m_Gizmo(std::make_unique<CTransformGizmo>(*m_RenderScene)),
  m_Icons(std::make_unique<CSceneViewportIcons>(*m_RenderScene)),
  m_Toolbar(ui::ELayoutDirection::Row),
  m_ModeCombo(m_Toolbar.Add(ui::CComboBox("Tool"))),
  m_SpaceCombo(m_Toolbar.Add(ui::CComboBox("Space"))),
  m_ChangedConnection(0)
{
  m_Sync->Update(*m_Document);
  m_RenderScene->MarkContentReplaced();
  UpdateHeadlight();

  m_ChangedConnection = m_Document->GetChangedSignal().Connect([this](const TSceneChange & _Change)
  {
    OnDocumentChanged(_Change);
  });

  SetupViewport();
  SetupToolbar();
}

CSceneWorkspace::~CSceneWorkspace()
{
  m_Document->GetChangedSignal().Disconnect(m_ChangedConnection);
}

//
// CEditorWorkspace
//

std::vector<core::TTypeKey> CSceneWorkspace::GetToolBoxes() const
{
  return {core::TypeOf<CSceneOutlinerWindow>(), core::TypeOf<CSceneObjectsWindow>()};
}

std::shared_ptr<IPropertySource> CSceneWorkspace::GetInspected() const
{
  if (m_Selection->GetNode().empty())
    return nullptr;

  return m_NodeProperties;
}

void CSceneWorkspace::OnActivated()
{
  m_Controller->SetDocument(m_Document, m_Selection);
  m_RenderScene->SetActive(true);
}

void CSceneWorkspace::OnDeactivated()
{
  m_Document->CommitPreview();
  m_Controller->ResetDocument(*m_Document);
  m_RenderScene->SetActive(false);
}

void CSceneWorkspace::OnGui()
{
  if (m_Sync->Update(*m_Document))
  {
    m_RenderScene->MarkContentChanged();
    UpdateHeadlight();
  }

  m_Sync->SetHighlightedNode(m_Selection->GetNode());

  HandleShortcuts();
  HandleEditShortcuts();
  DrawToolbar();

  m_Viewport->Draw("##SceneViewport");
}

//
// Implementation
//

void CSceneWorkspace::SetupViewport()
{
  m_Viewport->OnPick([this](const Ogre::Ray & _Ray)
  {
    m_Selection->Select(m_Sync->Pick(_Ray));
  });

  m_Viewport->OnOverlay([this](const TViewportFrame & _Frame)
  {
    return DrawOverlay(_Frame);
  });

  m_Gizmo->OnDrag([this](const Ogre::Affine3 & _Transform)
  {
    m_Document->PreviewWorldTransform(m_Selection->GetNode(), _Transform);
  });

  m_Gizmo->OnDragFinished([this]
  {
    m_Document->CommitPreview();
  });

  m_Gizmo->OnDragCanceled([this]
  {
    m_Document->CancelPreview();
  });
}

void CSceneWorkspace::SetupToolbar()
{
  m_ModeCombo.SetItems(std::vector<std::string>(ModeLabels.begin(), ModeLabels.end())).SetTooltip("Move (W), Rotate (E), Scale (R)").OnSelected([this](std::size_t _ModeIndex)
  {
    m_Gizmo->SetMode(static_cast<EGizmoMode>(_ModeIndex));
  });

  m_SpaceCombo.SetItems(std::vector<std::string>(SpaceLabels.begin(), SpaceLabels.end())).OnSelected([this](std::size_t _SpaceIndex)
  {
    m_Gizmo->SetSpace(static_cast<EGizmoSpace>(_SpaceIndex));
  });
}

void CSceneWorkspace::DrawToolbar()
{
  const float ComboWidth = ImGui::GetFontSize() * ComboWidthScale;

  m_ModeCombo.SetWidth(ComboWidth);
  m_ModeCombo.SetSelectedIndex(static_cast<std::size_t>(m_Gizmo->GetMode()));

  m_SpaceCombo.SetWidth(ComboWidth);
  m_SpaceCombo.SetSelectedIndex(static_cast<std::size_t>(m_Gizmo->GetSpace()));

  m_Toolbar.Draw();
}

void CSceneWorkspace::HandleShortcuts()
{
  const ImGuiIO & InputState = ImGui::GetIO();
  if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) || InputState.WantTextInput || InputState.KeyCtrl || ImGui::IsMouseDown(ImGuiMouseButton_Right))
    return;

  if (ImGui::IsKeyPressed(ImGuiKey_W, false))
    m_Gizmo->SetMode(EGizmoMode::Translate);

  if (ImGui::IsKeyPressed(ImGuiKey_E, false))
    m_Gizmo->SetMode(EGizmoMode::Rotate);

  if (ImGui::IsKeyPressed(ImGuiKey_R, false))
    m_Gizmo->SetMode(EGizmoMode::Scale);
}

void CSceneWorkspace::HandleEditShortcuts()
{
  const std::string & Selection = m_Selection->GetNode();

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_V))
    m_Controller->PasteNode(Selection, false);

  if (Selection.empty())
    return;

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_C))
    m_Controller->CopyNode(Selection);

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D))
    m_Controller->DuplicateNode(Selection);

  if (ImGui::Shortcut(ImGuiKey_Delete))
    m_Controller->DeleteNode(Selection);
}

bool CSceneWorkspace::DrawOverlay(const TViewportFrame & _Frame)
{
  HandleAssetDrop(_Frame);
  HandleTemplateDrop(_Frame);

  const std::string HoveredIcon = m_Icons->Draw(_Frame, m_Document->GetGraph(), m_Selection->GetNode());

  if (DrawGizmo(_Frame))
    return true;

  if (HoveredIcon.empty() || !_Frame.Input.IsClicked)
    return false;

  m_Selection->Select(HoveredIcon);
  return true;
}

void CSceneWorkspace::HandleAssetDrop(const TViewportFrame & _Frame)
{
  const auto AssetPath = GetAssetPayload().Peek();
  if (!AssetPath || !IsMouseInFrame(_Frame))
    return;

  const TViewportDropTarget Target{FindDropPosition(_Frame)};

  const auto Hint = m_Controller->GetDropHint(*AssetPath, Target);
  if (!Hint || !ImGui::BeginDragDropTarget())
    return;

  if (const auto Payload = GetAssetPayload().AcceptBeforeDelivery())
  {
    ImGui::SetTooltip("%s", Hint->c_str());

    if (Payload->IsDelivered)
      m_Controller->DropAsset(Payload->Data, Target);
  }

  ImGui::EndDragDropTarget();
}

void CSceneWorkspace::HandleTemplateDrop(const TViewportFrame & _Frame)
{
  if (!GetTemplatePayload().Peek() || !IsMouseInFrame(_Frame) || !ImGui::BeginDragDropTarget())
    return;

  if (const auto Payload = GetTemplatePayload().AcceptBeforeDelivery(); Payload && Payload->IsDelivered)
    m_Controller->CreateFromTemplate(Payload->Data, TViewportDropTarget{FindDropPosition(_Frame)});

  ImGui::EndDragDropTarget();
}

Ogre::Vector3 CSceneWorkspace::FindDropPosition(const TViewportFrame & _Frame) const
{
  const ImVec2    Mouse = ImGui::GetIO().MousePos;
  const Ogre::Ray Ray   = m_RenderScene->GetViewportRay((Mouse.x - _Frame.Origin.x) / _Frame.Size.x, (Mouse.y - _Frame.Origin.y) / _Frame.Size.y);

  if (const auto Distance = m_Sync->PickDistance(Ray))
    return Ray.getPoint(*Distance);

  const auto GroundHit = Ray.intersects(Ogre::Plane(Ogre::Vector3::UNIT_Y, 0.0f));
  if (GroundHit.first)
    return Ray.getPoint(GroundHit.second);

  return Ray.getPoint(m_RenderScene->GetCamera()->getDerivedPosition().length());
}

bool CSceneWorkspace::DrawGizmo(const TViewportFrame & _Frame)
{
  const std::string & Selection = m_Selection->GetNode();
  if (Selection.empty())
    return false;

  return m_Gizmo->Draw(_Frame, m_Document->GetGraph().GetWorldTransform(Selection));
}

void CSceneWorkspace::UpdateHeadlight()
{
  m_RenderScene->SetHeadlightEnabled(!m_Document->GetGraph().HasNodeOfType(CLightNode::TypeId));
}

//
// Events
//

void CSceneWorkspace::OnDocumentChanged(const TSceneChange & _Change)
{
  switch (_Change.Kind)
  {
    case ESceneChangeKind::Added:
    case ESceneChangeKind::Changed:
      m_Selection->Select(_Change.Name);
      break;

    case ESceneChangeKind::Renamed:
      if (m_Selection->GetNode() == _Change.Name)
        m_Selection->Select(_Change.NewName);
      break;

    case ESceneChangeKind::Removed:
      if (!m_Document->GetGraph().HasNode(m_Selection->GetNode()))
        m_Selection->Clear();
      break;
  }
}

} // namespace editor
