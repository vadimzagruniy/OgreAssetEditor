#include "AnimationTimelineWindow.h"
#include "core/ActionQueue.h"
#include "core/EditorService.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetNode.h"
#include "editor/assets/AssetRecord.h"
#include "editor/workspaces/EditorWorkspace.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "modules/animation/AnimationDocument.h"
#include "modules/animation/AnimationEditor.h"
#include "modules/animation/AnimationEditors.h"
#include "modules/scene/animation/ISceneAnimationHost.h"
#include "editor/timeline/presenter/TimelinePresenter.h"
#include "editor/timeline/views/TimelineView.h"
#include "ui/ChoiceModal.h"
#include "ui/ChoiceModalDesc.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace editor
{

namespace
{

constexpr float AnimationComboMinScale = 8.0f;

std::string MakeAnimationName(const CAnimationEditor & _Editor)
{
  const std::string DirtyMark = _Editor.GetDocument().IsDirty() ? " *" : "";

  return _Editor.GetDocument().GetRecord().RelativePath.stem().string() + DirtyMark;
}

std::string MakeAnimationLabel(const CAnimationEditor & _Editor)
{
  return MakeAnimationName(_Editor) + "###" + _Editor.GetDocument().GetRecord().RelativePath.generic_string();
}

} // namespace

//
// Constructor
//

CAnimationTimelineWindow::CAnimationTimelineWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<ui::CWindowManager> _WindowManager, std::shared_ptr<CAssetManager> _AssetManager, std::shared_ptr<core::CActionQueue> _ActionQueue):
  CDockedWindow(_Desc.Title, "AnimationTimeline", _Desc.Visible),
  m_WorkspaceManager(std::move(_WorkspaceManager)),
  m_WindowManager(std::move(_WindowManager)),
  m_AssetManager(std::move(_AssetManager)),
  m_ActionQueue(std::move(_ActionQueue))
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
    if (Workspace)
      DrawSceneAnimations(*Workspace);
    else
      ImGui::TextDisabled("Open a scene to edit animations.");

    return;
  }

  const auto Selected = SelectEditor(Editors);

  Selected->GetTimelineView().Draw(Selected->GetPresenter(), [this, &Editors, &Selected](float _Width) { DrawAnimationBar(Editors, Selected, _Width); });

  StopHiddenPreviews(Editors);
}

//
// Implementation
//

std::shared_ptr<CAnimationEditor> CAnimationTimelineWindow::SelectEditor(const std::vector<std::shared_ptr<CAnimationEditor>> & _Editors)
{
  if (!m_RequestedPath.empty())
  {
    m_SelectedPath = m_RequestedPath;
    m_RequestedPath.clear();
  }

  for (const auto & Editor : _Editors)
  {
    if (Editor->GetDocument().GetRecord().AbsolutePath == m_SelectedPath)
      return Editor;
  }

  m_SelectedPath = _Editors.front()->GetDocument().GetRecord().AbsolutePath;

  return _Editors.front();
}

void CAnimationTimelineWindow::DrawAnimationBar(const std::vector<std::shared_ptr<CAnimationEditor>> & _Editors, const std::shared_ptr<CAnimationEditor> & _Selected, float _Width)
{
  const float StartX = ImGui::GetCursorPosX();

  if (ImGui::Button("Close"))
    RequestClose(_Selected);

  ImGui::SetItemTooltip("Close this animation");

  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();

  const float UsedWidth = ImGui::GetCursorPosX() - StartX;
  ImGui::SetNextItemWidth(std::max(_Width - UsedWidth, ImGui::GetFontSize() * AnimationComboMinScale));

  const bool IsComboOpen = ImGui::BeginCombo("##Animation", MakeAnimationName(*_Selected).c_str());
  ImGui::SetItemTooltip("%s", _Selected->GetDocument().GetRecord().RelativePath.generic_string().c_str());

  if (!IsComboOpen)
    return;

  for (const auto & Editor : _Editors)
  {
    if (ImGui::Selectable(MakeAnimationLabel(*Editor).c_str(), Editor == _Selected))
      m_SelectedPath = Editor->GetDocument().GetRecord().AbsolutePath;
  }

  ImGui::EndCombo();
}

void CAnimationTimelineWindow::DrawSceneAnimations(const CEditorWorkspace & _Workspace) const
{
  const auto Host = _Workspace.GetServices().GetService<ISceneAnimationHost>();
  if (!Host)
  {
    ImGui::TextDisabled("Open a scene to edit animations.");
    return;
  }

  const auto Paths = Host->GetAnimations();
  if (Paths.empty())
  {
    ImGui::TextDisabled("Double-click an animation asset to edit it here.");
    return;
  }

  ImGui::TextUnformatted("Scene animations");

  for (const auto & Path : Paths)
  {
    ImGui::PushID(Path.c_str());

    const bool IsMissing = !m_AssetManager->HasNode(Path);

    ImGui::BeginDisabled(IsMissing);
    if (ImGui::SmallButton("Open"))
      OpenAnimation(Path);
    ImGui::EndDisabled();

    ImGui::SameLine();

    const std::string Label = IsMissing ? Path + " (missing)" : Path;

    ImGui::BeginDisabled(IsMissing);
    ImGui::Selectable(Label.c_str());
    ImGui::EndDisabled();

    if (ImGui::BeginPopupContextItem("##SceneAnimationMenu"))
    {
      if (ImGui::MenuItem("Remove from Scene"))
        Host->RemoveAnimation(Path);

      ImGui::EndPopup();
    }

    ImGui::PopID();
  }
}

void CAnimationTimelineWindow::OpenAnimation(const std::string & _Path) const
{
  m_ActionQueue->Post([AssetManager = m_AssetManager, WorkspaceManager = m_WorkspaceManager, _Path]
  {
    if (AssetManager->HasNode(_Path))
      WorkspaceManager->OpenAsset(AssetManager->GetNode(_Path).Record);
  });
}

void CAnimationTimelineWindow::StopHiddenPreviews(const std::vector<std::shared_ptr<CAnimationEditor>> & _Editors) const
{
  for (const auto & Editor : _Editors)
  {
    if (Editor->GetDocument().GetRecord().AbsolutePath != m_SelectedPath && Editor->GetPresenter().IsPreviewing())
      Editor->GetPresenter().Stop();
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
