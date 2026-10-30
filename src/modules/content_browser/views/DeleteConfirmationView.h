#pragma once

namespace editor
{

class IContentBrowserPresenter;

class CDeleteConfirmationView
{
public: // Constructor

  explicit CDeleteConfirmationView(IContentBrowserPresenter & _Presenter);

public: // Interface

  void Draw();

private: // Attributes

  IContentBrowserPresenter & m_Presenter;
};

} // namespace editor
