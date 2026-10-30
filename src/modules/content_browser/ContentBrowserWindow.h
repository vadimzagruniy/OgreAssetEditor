#pragma once

#include "ui/DockedWindow.h"

#include <memory>

namespace editor
{

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class CContentBrowserController;
class CContentToolbarView;
class CFolderTreeView;
class CNavigationBarView;
class CAssetListView;
class CAssetGridView;
class CDeleteConfirmationView;

class CContentBrowserWindow final:
  public ui::CDockedWindow
{
public: // Constructor/Destructor

  CContentBrowserWindow(const ui::TToolBoxDesc & _Desc, std::unique_ptr<CContentBrowserController> _Controller);

  ~CContentBrowserWindow() override;

private: // CWindow

  void OnGui() override;

private: // Attributes

  std::unique_ptr<CContentBrowserController> m_Controller;
  std::unique_ptr<CContentToolbarView>       m_ToolbarView;
  std::unique_ptr<CFolderTreeView>           m_FolderTreeView;
  std::unique_ptr<CNavigationBarView>        m_NavigationBarView;
  std::unique_ptr<CAssetListView>            m_AssetListView;
  std::unique_ptr<CAssetGridView>            m_AssetGridView;
  std::unique_ptr<CDeleteConfirmationView>   m_DeleteConfirmationView;
  float                                      m_SplitRatio;
};

} // namespace editor
