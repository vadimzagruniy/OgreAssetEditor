#pragma once

#include "ui/layout/Layout.h"

#include <filesystem>
#include <optional>

namespace editor
{

namespace ui
{
class CArrowButton;
class CButton;
class CChildArea;
} // namespace ui

class IContentBrowserPresenter;

class CNavigationBarView
{
public: // Constructor

  explicit CNavigationBarView(IContentBrowserPresenter & _Presenter);

public: // Interface

  void Draw();

private: // Implementation

  void RebuildBreadcrumbs(const std::filesystem::path & _Folder);

  void UpdateViewModeButton();

private: // Attributes

  IContentBrowserPresenter &           m_Presenter;
  ui::CLayout                          m_Layout;
  ui::CArrowButton &                   m_UpButton;
  ui::CChildArea &                     m_Breadcrumbs;
  ui::CButton &                        m_ViewModeButton;
  std::optional<std::filesystem::path> m_BreadcrumbsFolder;
};

} // namespace editor
