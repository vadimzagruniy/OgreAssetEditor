#pragma once

namespace editor
{

class IContentBrowserPresenter;

class CNavigationBarView
{
public: // Constructor

  explicit CNavigationBarView(IContentBrowserPresenter & _Presenter);

public: // Interface

  void Draw();

private: // Implementation

  void DrawBreadcrumbs(float _Width);

  void DrawViewModeToggle(float _Width);

private: // Attributes

  IContentBrowserPresenter & m_Presenter;
};

} // namespace editor
