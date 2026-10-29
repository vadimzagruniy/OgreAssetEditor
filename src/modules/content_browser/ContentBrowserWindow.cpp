#include "ContentBrowserWindow.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetRecord.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "ui/ToolBoxDesc.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor
//

CContentBrowserWindow::CContentBrowserWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CAssetManager> _AssetManager, std::shared_ptr<CWorkspaceManager> _WorkspaceManager):
  CDockedWindow(_Desc.Title, "ContentBrowser", _Desc.Visible),
  m_AssetManager(std::move(_AssetManager)),
  m_WorkspaceManager(std::move(_WorkspaceManager)),
  m_Filter{}
{
  // Empty
}

//
// CWindow
//

void CContentBrowserWindow::OnGui()
{
  ImGui::InputText("Filter", m_Filter, sizeof(m_Filter));

  if (ImGui::Button("Refresh"))
    m_AssetManager->Refresh();

  ImGui::Separator();

  if (!m_AssetManager->GetLastError().empty())
    ImGui::TextWrapped("%s", m_AssetManager->GetLastError().c_str());

  if (!ImGui::BeginChild("##AssetList", ImVec2(0, 0), ImGuiChildFlags_Borders))
  {
    ImGui::EndChild();
    return;
  }

  for (const auto & Asset : m_AssetManager->GetAssets())
  {
    const auto Label = Asset.RelativePath.generic_string();
    if (m_Filter[0] && Label.find(m_Filter) == std::string::npos)
      continue;

    if (ImGui::Selectable(Label.c_str(), m_Selected == Label))
      m_Selected = Label;

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
      m_WorkspaceManager->OpenAsset(Asset);
  }

  ImGui::EndChild();
}

} // namespace editor
