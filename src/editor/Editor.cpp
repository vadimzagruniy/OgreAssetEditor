#include "Editor.h"
#include "core/EditorService.h"
#include "core/Log.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModulesManager.h"
#include "editor/workspaces/WorkspaceController.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "editor/workspaces/WorkspaceWindow.h"
#include "ui/WindowManager.h"
#include "ui/dock_layout/DockLayoutBuilder.h"
#include "ui/dock_layout/DockLayoutDesc.h"
#include "ui/dock_layout/DockSlot.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CEditor::CEditor(std::filesystem::path _ProjectRoot):
  m_Log(std::make_shared<core::CLog>()),
  m_AssetManager(std::make_shared<CAssetManager>()),
  m_AssetTypes(std::make_shared<CAssetTypeRegistry>()),
  m_WorkspaceManager(std::make_shared<CWorkspaceManager>(m_AssetTypes, m_Log)),
  m_WindowManager(std::make_shared<ui::CWindowManager>()),
  m_WorkspaceController(std::make_unique<CWorkspaceController>(m_WorkspaceManager, m_WindowManager, m_Log)),
  m_WorkspaceWindow(std::make_unique<CWorkspaceWindow>(*m_WorkspaceManager, *m_WorkspaceController)),
  m_Services(std::make_unique<core::CEditorService>()),
  m_Modules(std::make_unique<CModulesManager>()),
  m_IsLayoutChecked(false),
  m_ResetLayoutRequested(false)
{
  m_AssetManager->OpenProject(_ProjectRoot);

  m_AssetManager->GetRemovedSignal().Connect([this](const std::filesystem::path & _Path)
  {
    m_WorkspaceManager->CloseAssets(_Path);
  });

  m_AssetManager->GetRenamedSignal().Connect([this](const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath)
  {
    m_WorkspaceManager->RelocateAssets(_OldPath, _NewPath, m_AssetManager->GetAssetsRoot());
  });

  m_Services->Register(m_AssetManager);
  m_Services->Register(m_AssetTypes);
  m_Services->Register(m_WorkspaceManager);
  m_Services->Register(m_WindowManager);
  m_Services->Register(m_Log);
}

CEditor::~CEditor() = default;

//
// Interface
//

bool CEditor::Initialize(std::string & _Error)
{
  if (!m_AssetManager->GetLastError().empty())
  {
    _Error = m_AssetManager->GetLastError();
    m_Log->Write(core::ELogLevel::Error, _Error);
    return false;
  }

  if (!m_Modules->Startup(*m_Services, _Error))
  {
    m_Log->Write(core::ELogLevel::Error, _Error);
    return false;
  }

  return true;
}

void CEditor::Shutdown() noexcept
{
  m_WorkspaceManager->CloseAll();
  m_Modules->Shutdown();
}

void CEditor::Tick(float _DeltaTime)
{
  m_Modules->Tick(_DeltaTime);
}

void CEditor::Draw()
{
  HandleShortcuts();

  DrawMainMenu();
  DrawDockSpace();

  m_WorkspaceWindow->Draw();
  m_WindowManager->Draw();
}

void CEditor::SetQuitCallback(std::function<void()> _Callback)
{
  m_QuitCallback = std::move(_Callback);
}

void CEditor::RequestQuit()
{
  m_WorkspaceController->ConfirmUnsavedChanges(m_QuitCallback);
}

//
// Services
//

std::shared_ptr<CAssetManager> CEditor::GetAssetManager() const noexcept
{
  return m_AssetManager;
}

std::shared_ptr<core::CLog> CEditor::GetLog() const noexcept
{
  return m_Log;
}

//
// Implementation
//

void CEditor::DrawMainMenu()
{
  if (!ImGui::BeginMainMenuBar())
    return;

  if (ImGui::BeginMenu("File"))
  {
    if (ImGui::MenuItem("Save", "Ctrl+S", false, m_WorkspaceController->CanSaveActive()))
      m_WorkspaceController->SaveActive();

    if (ImGui::MenuItem("Refresh Assets", "F5"))
      m_AssetManager->Refresh();

    if (ImGui::MenuItem("Close Active Asset"))
      m_WorkspaceController->RequestCloseActive();

    ImGui::Separator();

    if (ImGui::MenuItem("Exit", "Esc"))
      RequestQuit();

    ImGui::EndMenu();
  }

  DrawEditMenu();

  if (ImGui::BeginMenu("Window"))
  {
    m_WindowManager->DrawMenuItems();

    ImGui::Separator();

    if (ImGui::MenuItem("Reset Layout"))
      m_ResetLayoutRequested = true;

    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();
}

void CEditor::DrawEditMenu()
{
  if (!ImGui::BeginMenu("Edit"))
    return;

  const auto UndoName = m_WorkspaceController->GetUndoName();
  const auto RedoName = m_WorkspaceController->GetRedoName();

  if (ImGui::MenuItem(UndoName.empty() ? "Undo" : ("Undo " + UndoName).c_str(), "Ctrl+Z", false, !UndoName.empty()))
    m_WorkspaceController->UndoActive();

  if (ImGui::MenuItem(RedoName.empty() ? "Redo" : ("Redo " + RedoName).c_str(), "Ctrl+Y", false, !RedoName.empty()))
    m_WorkspaceController->RedoActive();

  ImGui::EndMenu();
}

void CEditor::HandleShortcuts()
{
  if (ImGui::Shortcut(ImGuiKey_F5, ImGuiInputFlags_RouteGlobal))
    m_AssetManager->Refresh();

  const bool IsPopupOpen = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);

  if (!IsPopupOpen && !ImGui::GetIO().WantTextInput && ImGui::Shortcut(ImGuiKey_Escape, ImGuiInputFlags_RouteGlobal))
    RequestQuit();

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
    m_WorkspaceController->SaveActive();

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, ImGuiInputFlags_RouteGlobal))
    m_WorkspaceController->UndoActive();

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y, ImGuiInputFlags_RouteGlobal) || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, ImGuiInputFlags_RouteGlobal))
    m_WorkspaceController->RedoActive();
}

void CEditor::DrawDockSpace()
{
  const ImGuiID DockSpaceId = ImGui::GetID("EditorDockSpace");
  const ImVec2  WorkSize    = ImGui::GetMainViewport()->WorkSize;

  // OGRE reports the viewport size to ImGui only after the first rendered frame.
  const bool HasViewportSize = WorkSize.x > 0.0f && WorkSize.y > 0.0f;

  if (HasViewportSize && !m_IsLayoutChecked)
  {
    m_IsLayoutChecked = true;

    if (!ui::CDockLayoutBuilder::HasLayout(DockSpaceId))
      m_ResetLayoutRequested = true;
  }

  if (HasViewportSize && m_ResetLayoutRequested)
  {
    m_ResetLayoutRequested = false;
    BuildDefaultLayout(DockSpaceId);
  }

  ImGui::DockSpaceOverViewport(DockSpaceId, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
}

void CEditor::BuildDefaultLayout(unsigned int _DockSpaceId)
{
  const ui::TDockLayoutDesc LayoutDesc
  {
    .LeftWidth        = 0.20f,
    .RightWidth       = 0.22f,
    .BottomHeight     = 0.25f,
    .SideBottomHeight = 0.50f
  };

  ui::CDockLayoutBuilder Builder(LayoutDesc);
  Builder.DockWindow(m_WorkspaceWindow->GetLabel(), ui::EDockSlot::Center);

  m_WindowManager->AddDefaultDocking(Builder);

  Builder.Build(_DockSpaceId);
}

} // namespace editor
