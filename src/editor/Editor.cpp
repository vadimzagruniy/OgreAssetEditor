#include "Editor.h"
#include "core/EditorService.h"
#include "core/Log.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModulesManager.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "ui/WindowManager.h"

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
  m_Services(std::make_unique<core::CEditorService>()),
  m_Modules(std::make_unique<CModulesManager>())
{
  m_AssetManager->OpenProject(_ProjectRoot);

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
  DrawMainMenu();

  ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

  m_WorkspaceManager->Draw();
  m_WindowManager->Draw();
}

void CEditor::SetQuitCallback(std::function<void()> _Callback)
{
  m_RequestQuit = std::move(_Callback);
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
    if (ImGui::MenuItem("Refresh Assets", "F5"))
      m_AssetManager->Refresh();

    if (ImGui::MenuItem("Close Active Asset"))
      m_WorkspaceManager->CloseActive();

    ImGui::Separator();

    if (ImGui::MenuItem("Exit", "Esc") && m_RequestQuit)
      m_RequestQuit();

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Window"))
  {
    m_WindowManager->DrawMenuItems();
    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();
}

} // namespace editor
