#pragma once

#include <memory>

namespace editor
{

namespace ui
{
class CWindowManager;
} // namespace ui

class IContentBrowserPresenter;

class CDeleteConfirmationView
{
public: // Constructor

  CDeleteConfirmationView(std::weak_ptr<IContentBrowserPresenter> _Presenter, ui::CWindowManager & _WindowManager);

public: // Interface

  void Draw();

private: // Implementation

  void OpenModal(const IContentBrowserPresenter & _Presenter);

private: // Attributes

  std::weak_ptr<IContentBrowserPresenter> m_Presenter;
  ui::CWindowManager &                    m_WindowManager;
  bool                                    m_IsModalOpened;
};

} // namespace editor
