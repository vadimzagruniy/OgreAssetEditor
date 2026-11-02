#include "NavigationBarView.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"
#include "ui/elements/ArrowButton.h"
#include "ui/elements/Button.h"
#include "ui/elements/Text.h"
#include "ui/layout/ChildArea.h"

#include <imgui.h>

#include <algorithm>
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
  m_Presenter(_Presenter),
  m_Layout(ui::ELayoutDirection::Row),
  m_UpButton(m_Layout.Add(ui::CArrowButton("##Up", ImGuiDir_Up))),
  m_Breadcrumbs(m_Layout.Add(ui::CChildArea("##Breadcrumbs", ui::ELayoutDirection::Row))),
  m_ViewModeButton(m_Layout.Add(ui::CButton(ListModeLabel)))
{
  m_UpButton.SetTooltip("Up").OnClick([this]
  {
    m_Presenter.NavigateUp();
  });

  m_Breadcrumbs.SetWindowFlags(ImGuiWindowFlags_NoScrollbar).SetScrollToEnd(true).SetWidth({ui::ESizeMode::Fill});

  m_ViewModeButton.SetTooltip("Switch view").OnClick([this]
  {
    const bool IsGrid = m_Presenter.GetViewMode() == EContentViewMode::Grid;
    m_Presenter.SetViewMode(IsGrid ? EContentViewMode::List : EContentViewMode::Grid);
  });
}

//
// Interface
//

void CNavigationBarView::Draw()
{
  const auto & CurrentFolder = m_Presenter.GetCurrentFolder();

  if (m_BreadcrumbsFolder != CurrentFolder)
    RebuildBreadcrumbs(CurrentFolder);

  m_UpButton.SetEnabled(!CurrentFolder.empty());
  UpdateViewModeButton();

  m_Layout.Draw();
}

//
// Implementation
//

void CNavigationBarView::RebuildBreadcrumbs(const std::filesystem::path & _Folder)
{
  m_BreadcrumbsFolder = _Folder;

  auto & Content = m_Breadcrumbs.GetContent();
  Content.Clear();

  Content.Add(ui::CButton("Assets")).OnClick([this]
  {
    m_Presenter.NavigateTo({});
  });

  const ImVec4          SeparatorColour = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
  std::filesystem::path Folder;
  int                   ElementIndex    = 0;

  for (const auto & Element : _Folder)
  {
    Folder /= Element;

    Content.Add(ui::CText(">")).SetColor(ImGuiCol_Text, SeparatorColour);
    Content.Add(ui::CButton(Element.string() + "##" + std::to_string(ElementIndex++))).OnClick([this, Folder]
    {
      m_Presenter.NavigateTo(Folder);
    });
  }

  m_Breadcrumbs.SetTooltip("Assets/" + _Folder.generic_string());
}

void CNavigationBarView::UpdateViewModeButton()
{
  const ImGuiStyle & Style      = ImGui::GetStyle();
  const float        LabelWidth = std::max(ImGui::CalcTextSize(ListModeLabel).x, ImGui::CalcTextSize(GridModeLabel).x);
  const bool         IsGrid     = m_Presenter.GetViewMode() == EContentViewMode::Grid;

  m_ViewModeButton.SetLabel(std::string(IsGrid ? GridModeLabel : ListModeLabel) + "###ViewMode").SetWidth(LabelWidth + Style.FramePadding.x * 2.0f);
}

} // namespace editor
