#pragma once

#include "ui/DockedWindow.h"

#include <memory>
#include <string>

namespace editor
{

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class CAssetManager;
class CWorkspaceManager;

class CContentBrowserWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CContentBrowserWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CAssetManager> _AssetManager, std::shared_ptr<CWorkspaceManager> _WorkspaceManager);

private: // CWindow

  void OnGui() override;

private: // Attributes

  std::shared_ptr<CAssetManager>     m_AssetManager;
  std::shared_ptr<CWorkspaceManager> m_WorkspaceManager;
  char                               m_Filter[256];
  std::string                        m_Selected;
};

} // namespace editor
