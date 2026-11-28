#pragma once

#include "ui/DockedWindow.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace editor
{

namespace core
{
class CActionQueue;
} // namespace core

namespace ui
{
class CWindowManager;
struct TToolBoxDesc;
} // namespace ui

class CAnimationEditor;
class CAssetDatabase;
class CEditorWorkspace;
class CWorkspaceManager;
struct TAssetReference;

class CAnimationTimelineWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CAnimationTimelineWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<ui::CWindowManager> _WindowManager, std::shared_ptr<const CAssetDatabase> _Assets, std::shared_ptr<core::CActionQueue> _ActionQueue);

public: // Interface

  void ShowAnimation(const std::filesystem::path & _AbsolutePath);

private: // CWindow

  void OnGui() override;

private: // Implementation

  std::shared_ptr<CAnimationEditor> SelectEditor(const std::vector<std::shared_ptr<CAnimationEditor>> & _Editors);

  void DrawAnimationBar(const std::vector<std::shared_ptr<CAnimationEditor>> & _Editors, const std::shared_ptr<CAnimationEditor> & _Selected, float _Width);

  void OpenTargetPicker(const std::shared_ptr<CAnimationEditor> & _Editor);

  void DrawSceneAnimations(const CEditorWorkspace & _Workspace) const;

  void OpenAnimation(const TAssetReference & _AnimationRef) const;

  void StopHiddenPreviews(const std::vector<std::shared_ptr<CAnimationEditor>> & _Editors) const;

  void RequestClose(const std::shared_ptr<CAnimationEditor> & _Editor) const;

private: // Attributes

  std::shared_ptr<CWorkspaceManager>    m_WorkspaceManager;
  std::shared_ptr<ui::CWindowManager>   m_WindowManager;
  std::shared_ptr<const CAssetDatabase> m_Assets;
  std::shared_ptr<core::CActionQueue>   m_ActionQueue;
  std::filesystem::path                 m_RequestedPath;
  std::filesystem::path                 m_SelectedPath;
  bool                                  m_IsTargetPickerOpened;
};

} // namespace editor
