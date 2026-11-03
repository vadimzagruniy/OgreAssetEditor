#include "SceneWorkspace.h"
#include "SceneDocument.h"
#include "SceneInstance.h"
#include "editor/render/RenderScene.h"
#include "editor/viewport/gizmo/TransformGizmo.h"
#include "editor/viewport/Viewport.h"
#include "presenter/SceneEditorController.h"
#include "render/SceneSync.h"
#include "ui/elements/ComboBox.h"
#include "views/SceneOutlinerWindow.h"

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

} // namespace

//
// Constructor/Destructor
//

CSceneWorkspace::CSceneWorkspace(std::unique_ptr<CSceneInstance> _Instance, std::shared_ptr<CSceneEditorController> _Controller, std::shared_ptr<core::CLog> _Log):
  CEditorWorkspace(std::move(_Instance)),
  m_Controller(std::move(_Controller)),
  m_Document(std::make_shared<CSceneDocument>(GetScene(), GetUndoStack())),
  m_RenderScene(std::make_unique<CRenderScene>()),
  m_Sync(std::make_unique<CSceneSync>(*m_RenderScene, std::move(_Log))),
  m_Viewport(std::make_unique<CViewport>(*m_RenderScene)),
  m_Gizmo(std::make_unique<CTransformGizmo>(*m_RenderScene)),
  m_Toolbar(ui::ELayoutDirection::Row),
  m_ModeCombo(m_Toolbar.Add(ui::CComboBox("Tool"))),
  m_SpaceCombo(m_Toolbar.Add(ui::CComboBox("Space")))
{
  m_Sync->Update(GetScene());
  m_RenderScene->MarkContentReplaced();

  SetupViewport();
  SetupToolbar();
}

CSceneWorkspace::~CSceneWorkspace() = default;

//
// CEditorWorkspace
//

std::vector<core::TTypeKey> CSceneWorkspace::GetToolBoxes() const
{
  return {core::TypeOf<CSceneOutlinerWindow>()};
}

void CSceneWorkspace::OnActivated()
{
  m_Controller->SetDocument(m_Document);
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
  m_Sync->Update(GetScene());
  m_Sync->SetHighlightedNode(m_Document->GetSelection());

  HandleShortcuts();
  DrawToolbar();

  m_Viewport->Draw("##SceneViewport");
}

bool CSceneWorkspace::OnSave(std::string & _Error)
{
  m_Document->CommitPreview();

  return GetScene().Save(_Error);
}

//
// Implementation
//

CSceneInstance & CSceneWorkspace::GetScene() const
{
  return *static_cast<CSceneInstance *>(GetInstance());
}

void CSceneWorkspace::SetupViewport()
{
  m_Viewport->OnPick([this](const Ogre::Ray & _Ray)
  {
    m_Document->Select(m_Sync->Pick(_Ray));
  });

  m_Viewport->OnOverlay([this](const TViewportFrame & _Frame)
  {
    return DrawGizmo(_Frame);
  });

  m_Gizmo->OnDrag([this](const Ogre::Affine3 & _Transform)
  {
    m_Document->PreviewWorldTransform(m_Document->GetSelection(), _Transform);
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

bool CSceneWorkspace::DrawGizmo(const TViewportFrame & _Frame)
{
  const std::string Selection = m_Document->GetSelection();
  if (Selection.empty())
    return false;

  return m_Gizmo->Draw(_Frame, GetScene().GetWorldTransform(Selection));
}

} // namespace editor
