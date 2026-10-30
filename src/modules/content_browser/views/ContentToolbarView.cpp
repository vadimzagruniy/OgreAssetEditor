#include "ContentToolbarView.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"

#include <imgui.h>


namespace editor
{

//
// Constructor
//

CContentToolbarView::CContentToolbarView(IContentBrowserPresenter & _Presenter):
  m_Presenter(_Presenter),
  m_FilterBuffer{}
{
  // Empty
}

//
// Interface
//

void CContentToolbarView::Draw()
{
  if (ImGui::Button("Refresh"))
    m_Presenter.Refresh();

  ImGui::SameLine();

  if (ImGui::Button("New Folder"))
    m_Presenter.CreateFolder();

  ImGui::SameLine();
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

  if (ImGui::InputTextWithHint("##Filter", "Filter", m_FilterBuffer, sizeof(m_FilterBuffer)))
    m_Presenter.SetFilter(m_FilterBuffer);
}

} // namespace editor
