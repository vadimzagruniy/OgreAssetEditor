#pragma once

#include "ui/Window.h"

#include <filesystem>

namespace editor
{

class CWorkspaceController;
class CWorkspaceManager;

class CWorkspaceWindow final:
  public ui::CWindow
{
public: // Constructor

  CWorkspaceWindow(CWorkspaceManager & _WorkspaceManager, CWorkspaceController & _WorkspaceController);

private: // CWindow

  void OnGui() override;

private: // Attributes

  CWorkspaceManager &    m_WorkspaceManager;
  CWorkspaceController & m_WorkspaceController;
  std::filesystem::path  m_ShownAssetPath;
};

} // namespace editor
