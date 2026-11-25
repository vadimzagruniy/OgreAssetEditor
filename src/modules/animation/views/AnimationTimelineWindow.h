#pragma once

#include "ui/DockedWindow.h"

#include <filesystem>
#include <memory>
#include <vector>

namespace editor
{

namespace ui
{
class CWindowManager;
struct TToolBoxDesc;
} // namespace ui

class CAnimationEditor;
class CWorkspaceManager;

class CAnimationTimelineWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CAnimationTimelineWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<ui::CWindowManager> _WindowManager);

public: // Interface

  void ShowAnimation(const std::filesystem::path & _AbsolutePath);

private: // CWindow

  void OnGui() override;

private: // Implementation

  void StopHiddenPreviews(const std::vector<std::shared_ptr<CAnimationEditor>> & _Editors) const;

  void RequestClose(const std::shared_ptr<CAnimationEditor> & _Editor) const;

private: // Attributes

  std::shared_ptr<CWorkspaceManager>  m_WorkspaceManager;
  std::shared_ptr<ui::CWindowManager> m_WindowManager;
  std::filesystem::path               m_RequestedPath;
  std::filesystem::path               m_SelectedPath;
};

} // namespace editor
