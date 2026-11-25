#pragma once

#include <memory>

namespace editor
{

namespace ui
{
class CWindowManager;
} // namespace ui

class IContentBrowserPresenter;

class CCreateAssetView
{
public: // Constructor

  CCreateAssetView(std::weak_ptr<IContentBrowserPresenter> _Presenter, ui::CWindowManager & _WindowManager);

public: // Interface

  void Draw();

private: // Attributes

  std::weak_ptr<IContentBrowserPresenter> m_Presenter;
  ui::CWindowManager &                    m_WindowManager;
  bool                                    m_IsModalOpened;
};

} // namespace editor
