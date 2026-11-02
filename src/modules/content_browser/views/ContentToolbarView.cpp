#include "ContentToolbarView.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"
#include "ui/elements/Button.h"
#include "ui/elements/InputText.h"

#include <string>

namespace editor
{

//
// Constructor
//

CContentToolbarView::CContentToolbarView(IContentBrowserPresenter & _Presenter):
  m_Presenter(_Presenter),
  m_Layout(ui::ELayoutDirection::Row)
{
  m_Layout.Add(ui::CButton("Refresh")).OnClick([this]
  {
    m_Presenter.Refresh();
  });

  m_Layout.Add(ui::CButton("New Folder")).OnClick([this]
  {
    m_Presenter.CreateFolder();
  });

  m_Layout.Add(ui::CInputText("##Filter")).SetHint("Filter").SetWidth({ui::ESizeMode::Fill}).OnChanged([this](const std::string & _Filter)
  {
    m_Presenter.SetFilter(_Filter);
  });
}

//
// Interface
//

void CContentToolbarView::Draw()
{
  m_Layout.Draw();
}

} // namespace editor
