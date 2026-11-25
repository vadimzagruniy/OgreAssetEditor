#include "AnimationTimelineWindow.h"
#include "editor/assets/AssetRecord.h"
#include "editor/workspaces/EditorWorkspace.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "modules/animation/AnimationDocument.h"
#include "modules/animation/AnimationEditor.h"
#include "modules/animation/AnimationEditors.h"
#include "editor/timeline/playback/TimelinePlayer.h"
#include "editor/timeline/views/TimelineView.h"
#include "ui/ChoiceModal.h"
#include "ui/ChoiceModalDesc.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"

#include <imgui.h>

#include <string>
#include <utility>
#include <vector>

namespace editor
{

namespace
{

std::string MakeTabLabel(const CAnimationEditor & _Editor)
{
  const auto &      Record    = _Editor.GetDocument().GetRecord();
  const std::string DirtyMark = _Editor.GetDocument().IsDirty() ? " *" : "";

  return Record.RelativePath.stem().string() + DirtyMark + "###" + Record.RelativePath.generic_string();
}

} // namespace

//
// Constructor
//

CAnimationTimelineWindow::CAnimationTimelineWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<ui::CWindowManager> _WindowManager):
  CDockedWindow(_Desc.Title, "AnimationTimeline", _Desc.Visible),
  m_WorkspaceManager(std::move(_WorkspaceManager)),
  m_WindowManager(std::move(_WindowManager))
{
  // Empty
}

//
// Interface
//

void CAnimationTimelineWindow::ShowAnimation(const std::filesystem::path & _AbsolutePath)
{
  m_RequestedPath = _AbsolutePath;
  RequestFocus();
}

//
// CWindow
//

void CAnimationTimelineWindow::OnGui()
{
  const auto * Workspace = m_WorkspaceManager->GetActive();
  const auto   Editors   = Workspace ? FindAnimationEditors(*Workspace) : std::vector<std::shared_ptr<CAnimationEditor>>();

  if (Editors.empty())
  {
    ImGui::TextDisabled("Double-click an animation asset to edit it here.");
    return;
  }

  if (!ImGui::BeginTabBar("##Animations"))
    return;

  std::shared_ptr<CAnimationEditor> ClosedEditor;

  for (const auto & Editor : Editors)
  {
    const bool              IsRequested = Editor->GetDocument().GetRecord().AbsolutePath == m_RequestedPath;
    const ImGuiTabItemFlags TabFlags    = IsRequested ? ImGuiTabItemFlags_SetSelected : 0;
    bool                    IsOpen      = true;

    if (ImGui::BeginTabItem(MakeTabLabel(*Editor).c_str(), &IsOpen, TabFlags))
    {
      m_SelectedPath = Editor->GetDocument().GetRecord().AbsolutePath;
      Editor->GetTimelineView().Draw(Editor->GetAnimation().GetTimeline(), Editor->GetPlayer());
      ImGui::EndTabItem();
    }

    if (!IsOpen)
      ClosedEditor = Editor;
  }

  ImGui::EndTabBar();

  m_RequestedPath.clear();

  StopHiddenPreviews(Editors);

  if (ClosedEditor)
    RequestClose(ClosedEditor);
}

//
// Implementation
//

void CAnimationTimelineWindow::StopHiddenPreviews(const std::vector<std::shared_ptr<CAnimationEditor>> & _Editors) const
{
  for (const auto & Editor : _Editors)
  {
    if (Editor->GetDocument().GetRecord().AbsolutePath != m_SelectedPath && Editor->GetPlayer().IsPreviewing())
      Editor->GetPlayer().Stop();
  }
}

void CAnimationTimelineWindow::RequestClose(const std::shared_ptr<CAnimationEditor> & _Editor) const
{
  const auto * Host = m_WorkspaceManager->GetActive();
  if (!Host)
    return;

  auto Detach = [WorkspaceManager = m_WorkspaceManager, HostPath = Host->GetDocument().GetRecord().AbsolutePath, WeakEditor = std::weak_ptr<CAnimationEditor>(_Editor)](bool _IsSaving, std::string & _Error)
  {
    const auto Editor    = WeakEditor.lock();
    const auto HostIndex = WorkspaceManager->FindWorkspace(HostPath);

    if (!Editor || !HostIndex)
      return true;

    if (_IsSaving && !Editor->GetDocument().Save(_Error))
      return false;

    WorkspaceManager->DetachEditor(*HostIndex, *Editor);

    return true;
  };

  if (!_Editor->GetDocument().IsDirty())
  {
    std::string Error;
    Detach(false, Error);
    return;
  }

  ui::TChoiceModalDesc Desc;
  Desc.Title   = "Unsaved Changes";
  Desc.Message = "Save changes to " + _Editor->GetDocument().GetRecord().RelativePath.filename().string() + " before closing?";
  Desc.Choices =
  {
    {"Save",       [Detach](std::string & _Error) { return Detach(true, _Error); }},
    {"Don't Save", [Detach](std::string & _Error) { return Detach(false, _Error); }},
    {"Cancel",     {}}
  };

  m_WindowManager->OpenModal(std::make_unique<ui::CChoiceModal>(std::move(Desc)));
}

} // namespace editor
