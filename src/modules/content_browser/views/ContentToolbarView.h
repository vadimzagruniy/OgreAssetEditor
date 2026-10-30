#pragma once

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
  char                       m_FilterBuffer[128];
};

} // namespace editor
