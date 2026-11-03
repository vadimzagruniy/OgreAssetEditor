#include "ContentToolbarView.h"
#include "editor/assets/AssetFactoryInfo.h"
#include "modules/content_browser/presenter/IContentBrowserPresenter.h"
#include "ui/elements/Button.h"
#include "ui/elements/InputText.h"

#include <cstddef>
#include <string>
#include <utility>

namespace editor
{

//
// Constructor
//

CContentToolbarView::CContentToolbarView(IContentBrowserPresenter & _Presenter):
  m_Presenter(_Presenter),
  m_Layout(ui::ELayoutDirection::Row),
  m_CreateButton(m_Layout.Add(ui::CButton("Create"))),
  m_CreateMenu("ContentCreate")
{
  m_CreateButton.OnClick([this]
  {
    m_CreateMenu.Open();
  });

  m_CreateMenu.OnSelected([this](std::size_t _FactoryIndex)
  {
    m_Presenter.CreateAsset(m_FactoryIdentifiers[_FactoryIndex]);
  });

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
  UpdateCreateMenu();

  m_Layout.Draw();
  m_CreateMenu.Draw();
}

//
// Implementation
//

void CContentToolbarView::UpdateCreateMenu()
{
  std::vector<std::string> Labels;
  m_FactoryIdentifiers.clear();

  for (const auto & Factory : m_Presenter.GetAssetFactories())
  {
    Labels.push_back(Factory.Label);
    m_FactoryIdentifiers.push_back(Factory.Identifier);
  }

  m_CreateButton.SetEnabled(!Labels.empty());
  m_CreateMenu.SetItems(std::move(Labels));
}

} // namespace editor
