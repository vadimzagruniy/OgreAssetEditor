#pragma once

#include "ui/DockedWindow.h"

#include <memory>

namespace editor
{

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class CWorkspaceManager;

class CPropertiesWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CPropertiesWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CWorkspaceManager> _WorkspaceManager);

private: // CWindow

  void OnGui() override;

private: // Attributes

  std::shared_ptr<CWorkspaceManager> m_WorkspaceManager;
};

} // namespace editor
