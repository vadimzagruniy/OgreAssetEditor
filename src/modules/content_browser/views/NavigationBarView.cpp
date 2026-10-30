#include "NavigationBarView.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"

#include <imgui.h>

#include <algorithm>
#include <filesystem>
#include <string>

namespace editor
{

namespace
{

constexpr const char * ListModeLabel = "List";
constexpr const char * GridModeLabel = "Grid";

} // namespace

//
// Constructor
//

CNavigationBarView::CNavigationBarView(IContentBrowserPresenter & _Presenter):
  m_Presenter(_Presenter)
{
  // Empty
}

//
// Interface
//

void CNavigationBarView::Draw()
{
  const ImGuiStyle & Style       = ImGui::GetStyle();
  const float        LabelWidth  = std::max(ImGui::CalcTextSize(ListModeLabel).x, ImGui::CalcTextSize(GridModeLabel).x);
  const float        ToggleWidth = LabelWidth + Style.FramePadding.x * 2.0f;

  ImGui::BeginDisabled(m_Presenter.GetCurrentFolder().empty());

  if (ImGui::ArrowButton("##Up", ImGuiDir_Up))
    m_Presenter.NavigateUp();

  ImGui::EndDisabled();
  ImGui::SetItemTooltip("Up");

  ImGui::SameLine();

  DrawBreadcrumbs(ImGui::GetContentRegionAvail().x - ToggleWidth - Style.ItemSpacing.x);

  ImGui::SameLine();

  DrawViewModeToggle(ToggleWidth);
}

//
// Implementation
//

void CNavigationBarView::DrawBreadcrumbs(float _Width)
{
  const auto & CurrentFolder = m_Presenter.GetCurrentFolder();

  if (ImGui::BeginChild("##Breadcrumbs", ImVec2(_Width, ImGui::GetFrameHeight()), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar))
  {
    if (ImGui::Button("Assets"))
      m_Presenter.NavigateTo({});

    std::filesystem::path Folder;
    int                   ElementIndex = 0;

    for (const auto & Element : CurrentFolder)
    {
      Folder /= Element;

      ImGui::SameLine();
      ImGui::TextDisabled(">");
      ImGui::SameLine();

      ImGui::PushID(ElementIndex++);

      if (ImGui::Button(Element.string().c_str()))
        m_Presenter.NavigateTo(Folder);

      ImGui::PopID();
    }

    ImGui::SetScrollHereX(1.0f);
  }

  ImGui::EndChild();
  ImGui::SetItemTooltip("Assets/%s", CurrentFolder.generic_string().c_str());
}

void CNavigationBarView::DrawViewModeToggle(float _Width)
{
  const bool   IsGrid = m_Presenter.GetViewMode() == EContentViewMode::Grid;
  const char * Label  = IsGrid ? GridModeLabel : ListModeLabel;

  if (ImGui::Button(Label, ImVec2(_Width, 0.0f)))
    m_Presenter.SetViewMode(IsGrid ? EContentViewMode::List : EContentViewMode::Grid);

  ImGui::SetItemTooltip("Switch view");
}

} // namespace editor
