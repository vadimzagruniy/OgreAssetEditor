#include "WorkspaceWindow.h"
#include "AssetInstance.h"
#include "EditorWorkspace.h"
#include "WorkspaceManager.h"
#include "editor/assets/AssetRecord.h"

#include <imgui.h>

namespace editor
{

namespace
{

const std::filesystem::path & GetAssetPath(const CEditorWorkspace & _Workspace)
{
  return _Workspace.GetInstance()->GetRecord().RelativePath;
}

} // namespace

//
// Constructor
//

CWorkspaceWindow::CWorkspaceWindow(CWorkspaceManager & _WorkspaceManager):
  CWindow("Resource Editors", "WorkspaceHost"),
  m_WorkspaceManager(_WorkspaceManager)
{
  SetTitleBarHidden(true);
}

//
// CWindow
//

void CWorkspaceWindow::OnGui()
{
  if (!ImGui::BeginTabBar("##OpenAssetTabs"))
    return;

  const auto &      Workspaces  = m_WorkspaceManager.GetWorkspaces();
  const std::size_t ActiveIndex = m_WorkspaceManager.GetActiveIndex();
  const bool        HasActive   = m_WorkspaceManager.HasActive();

  const bool IsSelectionForced = HasActive && GetAssetPath(*Workspaces[ActiveIndex]) != m_ShownAssetPath;

  if (IsSelectionForced)
    m_ShownAssetPath = GetAssetPath(*Workspaces[ActiveIndex]);

  for (std::size_t WorkspaceIndex = 0; WorkspaceIndex < Workspaces.size(); ++WorkspaceIndex)
  {
    auto &                  Workspace = *Workspaces[WorkspaceIndex];
    bool                    IsOpen    = true;
    const ImGuiTabItemFlags TabFlags  = IsSelectionForced && WorkspaceIndex == ActiveIndex ? ImGuiTabItemFlags_SetSelected : 0;

    if (ImGui::BeginTabItem(Workspace.GetTitle().c_str(), &IsOpen, TabFlags))
    {
      if (!IsSelectionForced)
      {
        m_WorkspaceManager.SetActive(WorkspaceIndex);
        m_ShownAssetPath = GetAssetPath(Workspace);
      }

      Workspace.OnGui();

      ImGui::EndTabItem();
    }

    if (!IsOpen)
    {
      m_WorkspaceManager.Close(WorkspaceIndex);
      break;
    }
  }

  ImGui::EndTabBar();
}

} // namespace editor
