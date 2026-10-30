#pragma once

#include "ui/Window.h"

#include <filesystem>

namespace editor
{

class CWorkspaceManager;

class CWorkspaceWindow final:
  public ui::CWindow
{
public: // Constructor

  explicit CWorkspaceWindow(CWorkspaceManager & _WorkspaceManager);

private: // CWindow

  void OnGui() override;

private: // Attributes

  CWorkspaceManager &   m_WorkspaceManager;
  std::filesystem::path m_ShownAssetPath;
};

} // namespace editor
