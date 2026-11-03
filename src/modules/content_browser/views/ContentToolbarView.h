#pragma once

#include "ui/PopupMenu.h"
#include "ui/layout/Layout.h"

#include <string>
#include <vector>

namespace editor
{

namespace ui
{
class CButton;
} // namespace ui

class IContentBrowserPresenter;

class CContentToolbarView
{
public: // Constructor

  explicit CContentToolbarView(IContentBrowserPresenter & _Presenter);

public: // Interface

  void Draw();

private: // Implementation

  void UpdateCreateMenu();

private: // Attributes

  IContentBrowserPresenter & m_Presenter;
  ui::CLayout                m_Layout;
  ui::CButton &              m_CreateButton;
  ui::CPopupMenu             m_CreateMenu;
  std::vector<std::string>   m_FactoryIdentifiers;
};

} // namespace editor
