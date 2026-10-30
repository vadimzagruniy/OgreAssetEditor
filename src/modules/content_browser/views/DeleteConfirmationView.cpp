#include "DeleteConfirmationView.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"

#include <imgui.h>

#include <cstddef>

namespace editor
{

namespace
{

constexpr const char * PopupName       = "Delete Assets";
constexpr std::size_t  MaxListedPaths  = 8;

} // namespace

//
// Constructor
//

CDeleteConfirmationView::CDeleteConfirmationView(IContentBrowserPresenter & _Presenter):
  m_Presenter(_Presenter)
{
  // Empty
}

//
// Interface
//

void CDeleteConfirmationView::Draw()
{
  if (m_Presenter.IsDeleteRequested() && !ImGui::IsPopupOpen(PopupName))
    ImGui::OpenPopup(PopupName);

  if (!ImGui::BeginPopupModal(PopupName, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    return;

  const auto & Selection = m_Presenter.GetSelection();

  ImGui::Text("Delete %zu item(s)? This cannot be undone.", Selection.size());
  ImGui::Separator();

  std::size_t ListedCount = 0;

  for (const auto & Path : Selection)
  {
    if (ListedCount++ == MaxListedPaths)
    {
      ImGui::TextDisabled("...");
      break;
    }

    ImGui::BulletText("%s", Path.generic_string().c_str());
  }

  ImGui::Separator();

  if (ImGui::Button("Delete"))
  {
    m_Presenter.ConfirmDelete();
    ImGui::CloseCurrentPopup();
  }

  ImGui::SameLine();

  if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
  {
    m_Presenter.CancelDelete();
    ImGui::CloseCurrentPopup();
  }

  ImGui::EndPopup();
}

} // namespace editor
