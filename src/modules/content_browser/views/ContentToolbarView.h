#pragma once

#include "ui/layout/Layout.h"

namespace editor
{

class IContentBrowserPresenter;

class CContentToolbarView
{
public: // Constructor

  explicit CContentToolbarView(IContentBrowserPresenter & _Presenter);

public: // Interface

  void Draw();

private: // Attributes

  IContentBrowserPresenter & m_Presenter;
  ui::CLayout                m_Layout;
};

} // namespace editor
