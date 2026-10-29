#include "PropertiesWindow.h"
#include "editor/assets/AssetRecord.h"
#include "editor/workspaces/AssetInstance.h"
#include "editor/workspaces/AssetProperty.h"
#include "editor/workspaces/EditorWorkspace.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "ui/ToolBoxDesc.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor
//

CPropertiesWindow::CPropertiesWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CWorkspaceManager> _WorkspaceManager):
  CDockedWindow(_Desc.Title, "Properties", _Desc.Visible),
  m_WorkspaceManager(std::move(_WorkspaceManager))
{
  // Empty
}

//
// CWindow
//

void CPropertiesWindow::OnGui()
{
  const auto * Workspace = m_WorkspaceManager->GetActive();
  if (!Workspace || !Workspace->GetInstance())
  {
    ImGui::TextUnformatted("No asset selected");
    return;
  }

  const auto * Instance = Workspace->GetInstance();

  ImGui::Text("Type: %s", Instance->GetTypeName().c_str());
  ImGui::Text("Path: %s", Instance->GetRecord().RelativePath.generic_string().c_str());

  ImGui::Separator();

  for (const auto & Property : Instance->GetProperties())
    ImGui::Text("%s: %s", Property.Name.c_str(), Property.Value.c_str());
}

} // namespace editor
