#include "ContentBrowserWindow.h"
#include "presenter/ContentBrowserController.h"
#include "ui/ToolBoxDesc.h"
#include "ui/widgets/Splitter.h"
#include "views/AssetGridView.h"
#include "views/AssetListView.h"
#include "views/ContentToolbarView.h"
#include "views/DeleteConfirmationView.h"
#include "views/FolderTreeView.h"
#include "views/NavigationBarView.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CContentBrowserWindow::CContentBrowserWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CContentBrowserController> _Controller, ui::CWindowManager & _WindowManager):
  CDockedWindow(_Desc.Title, "ContentBrowser", _Desc.Visible),
  m_Controller(std::move(_Controller)),
  m_ToolbarView(std::make_unique<CContentToolbarView>(*m_Controller)),
  m_FolderTreeView(std::make_unique<CFolderTreeView>(*m_Controller)),
  m_NavigationBarView(std::make_unique<CNavigationBarView>(*m_Controller)),
  m_AssetListView(std::make_unique<CAssetListView>(*m_Controller)),
  m_AssetGridView(std::make_unique<CAssetGridView>(*m_Controller)),
  m_DeleteConfirmationView(std::make_unique<CDeleteConfirmationView>(m_Controller, _WindowManager)),
  m_SplitRatio(0.4f)
{
  // Empty
}

CContentBrowserWindow::~CContentBrowserWindow() = default;

//
// CWindow
//

void CContentBrowserWindow::OnGui()
{
  m_ToolbarView->Draw();

  ImGui::Separator();

  const float AvailableHeight = ImGui::GetContentRegionAvail().y;

  if (ImGui::BeginChild("##FolderTree", ImVec2(0.0f, AvailableHeight * m_SplitRatio)))
    m_FolderTreeView->Draw();

  ImGui::EndChild();

  ui::HorizontalSplitter("##ContentSplitter", m_SplitRatio, AvailableHeight, 0.1f, 0.9f);

  m_NavigationBarView->Draw();

  ImGui::Separator();

  if (m_Controller->GetViewMode() == EContentViewMode::Grid)
    m_AssetGridView->Draw();
  else
    m_AssetListView->Draw();

  m_DeleteConfirmationView->Draw();

  m_Controller->ApplyPendingActions();
}

} // namespace editor
