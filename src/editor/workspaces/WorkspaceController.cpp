#include "WorkspaceController.h"
#include "EditorWorkspace.h"
#include "IAttachedEditor.h"
#include "WorkspaceManager.h"
#include "core/Log.h"
#include "editor/assets/AssetRecord.h"
#include "editor/documents/AssetDocument.h"
#include "ui/ChoiceModal.h"
#include "ui/ChoiceModalDesc.h"
#include "ui/DockedWindow.h"
#include "ui/WindowManager.h"

#include <algorithm>
#include <filesystem>
#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CWorkspaceController::CWorkspaceController(std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<ui::CWindowManager> _WindowManager, std::shared_ptr<core::CLog> _Log):
  m_WorkspaceManager(std::move(_WorkspaceManager)),
  m_WindowManager(std::move(_WindowManager)),
  m_Log(std::move(_Log)),
  m_ActiveChangedConnection(0),
  m_AttachedChangedConnection(0)
{
  m_ActiveChangedConnection = m_WorkspaceManager->GetActiveChangedSignal().Connect([this]
  {
    UpdateToolBoxes();
  });

  m_AttachedChangedConnection = m_WorkspaceManager->GetAttachedChangedSignal().Connect([this]
  {
    UpdateToolBoxes();
  });
}

CWorkspaceController::~CWorkspaceController()
{
  m_WorkspaceManager->GetActiveChangedSignal().Disconnect(m_ActiveChangedConnection);
  m_WorkspaceManager->GetAttachedChangedSignal().Disconnect(m_AttachedChangedConnection);
}

//
// Interface
//

bool CWorkspaceController::CanSaveActive() const
{
  return m_WorkspaceManager->HasActive() && m_WorkspaceManager->GetActive()->IsDirty();
}

void CWorkspaceController::SaveActive()
{
  if (!CanSaveActive())
    return;

  auto &      Workspace = *m_WorkspaceManager->GetActive();
  std::string Error;

  if (Workspace.Save(Error))
  {
    m_Log->Write(core::ELogLevel::Info, "Saved " + Workspace.GetTitle());
    return;
  }

  m_Log->Write(core::ELogLevel::Error, Error);

  ui::TChoiceModalDesc Desc;
  Desc.Title   = "Save Failed";
  Desc.Message = Error;
  Desc.Choices = {{"Close", {}}};

  m_WindowManager->OpenModal(std::make_unique<ui::CChoiceModal>(std::move(Desc)));
}

std::string CWorkspaceController::GetUndoName() const
{
  return m_WorkspaceManager->HasActive() ? GetFocusedDocument(*m_WorkspaceManager->GetActive()).GetHistory().GetUndoName() : std::string();
}

std::string CWorkspaceController::GetRedoName() const
{
  return m_WorkspaceManager->HasActive() ? GetFocusedDocument(*m_WorkspaceManager->GetActive()).GetHistory().GetRedoName() : std::string();
}

void CWorkspaceController::UndoActive()
{
  if (m_WorkspaceManager->HasActive())
    GetFocusedDocument(*m_WorkspaceManager->GetActive()).GetHistory().Undo();
}

void CWorkspaceController::RedoActive()
{
  if (m_WorkspaceManager->HasActive())
    GetFocusedDocument(*m_WorkspaceManager->GetActive()).GetHistory().Redo();
}

void CWorkspaceController::RequestClose(std::size_t _Index)
{
  const auto & Workspace = *m_WorkspaceManager->GetWorkspaces().at(_Index);

  if (!Workspace.IsDirty())
  {
    m_WorkspaceManager->Close(_Index);
    return;
  }

  auto CloseWorkspace = [WorkspaceManager = m_WorkspaceManager, AssetPath = Workspace.GetDocument().GetRecord().AbsolutePath](bool _IsSaving, std::string & _Error)
  {
    const auto WorkspaceIndex = WorkspaceManager->FindWorkspace(AssetPath);
    if (!WorkspaceIndex)
      return true;

    if (_IsSaving && !WorkspaceManager->GetWorkspaces()[*WorkspaceIndex]->Save(_Error))
      return false;

    WorkspaceManager->Close(*WorkspaceIndex);

    return true;
  };

  ui::TChoiceModalDesc Desc;
  Desc.Title   = "Unsaved Changes";
  Desc.Message = "Save changes to " + Workspace.GetTitle() + " before closing?";
  Desc.Choices =
  {
    {"Save",       [CloseWorkspace](std::string & _Error) { return CloseWorkspace(true, _Error); }},
    {"Don't Save", [CloseWorkspace](std::string & _Error) { return CloseWorkspace(false, _Error); }},
    {"Cancel",     {}}
  };

  m_WindowManager->OpenModal(std::make_unique<ui::CChoiceModal>(std::move(Desc)));
}

void CWorkspaceController::RequestCloseActive()
{
  if (m_WorkspaceManager->HasActive())
    RequestClose(m_WorkspaceManager->GetActiveIndex());
}

void CWorkspaceController::ConfirmUnsavedChanges(std::function<void()> _OnConfirmed)
{
  if (!m_WorkspaceManager->HasDirtyWorkspaces())
  {
    if (_OnConfirmed)
      _OnConfirmed();

    return;
  }

  ui::TChoiceModalDesc Desc;
  Desc.Title   = "Unsaved Changes";
  Desc.Message = "Some assets have unsaved changes. Save them before exit?";
  Desc.Choices =
  {
    {"Save All", [WorkspaceManager = m_WorkspaceManager, OnConfirmed = _OnConfirmed](std::string & _Error)
    {
      if (!WorkspaceManager->SaveAll(_Error))
        return false;

      if (OnConfirmed)
        OnConfirmed();

      return true;
    }},
    {"Don't Save", [OnConfirmed = _OnConfirmed](std::string & /*_Error*/)
    {
      if (OnConfirmed)
        OnConfirmed();

      return true;
    }},
    {"Cancel", {}}
  };

  m_WindowManager->OpenModal(std::make_unique<ui::CChoiceModal>(std::move(Desc)));
}

//
// Implementation
//

CAssetDocument & CWorkspaceController::GetFocusedDocument(const CEditorWorkspace & _Workspace) const
{
  std::shared_ptr<IAttachedEditor> FocusedEditor;
  int                              FocusFrame = std::max(_Workspace.GetFocusFrame(), GetFocusFrame(_Workspace.GetToolBoxes()));

  for (const auto & Editor : _Workspace.GetAttachedEditors())
  {
    const int EditorFocusFrame = GetFocusFrame(Editor->GetToolBoxes());
    if (EditorFocusFrame <= FocusFrame)
      continue;

    FocusedEditor = Editor;
    FocusFrame    = EditorFocusFrame;
  }

  return FocusedEditor ? FocusedEditor->GetDocument() : _Workspace.GetDocument();
}

int CWorkspaceController::GetFocusFrame(const std::vector<core::TTypeKey> & _ToolBoxes) const
{
  int FocusFrame = -1;

  for (const auto & ToolBox : _ToolBoxes)
  {
    if (const auto Window = m_WindowManager->GetToolBox(ToolBox))
      FocusFrame = std::max(FocusFrame, Window->GetFocusFrame());
  }

  return FocusFrame;
}

void CWorkspaceController::UpdateToolBoxes()
{
  std::vector<core::TTypeKey> ToolBoxes;

  if (m_WorkspaceManager->HasActive())
  {
    const auto & Workspace = *m_WorkspaceManager->GetActive();
    ToolBoxes = Workspace.GetToolBoxes();

    for (const auto & Editor : Workspace.GetAttachedEditors())
    {
      const auto EditorToolBoxes = Editor->GetToolBoxes();
      ToolBoxes.insert(ToolBoxes.end(), EditorToolBoxes.begin(), EditorToolBoxes.end());
    }
  }

  for (const auto & ToolBox : m_ShownToolBoxes)
  {
    if (std::find(ToolBoxes.begin(), ToolBoxes.end(), ToolBox) == ToolBoxes.end())
      m_WindowManager->HideToolBox(ToolBox);
  }

  for (const auto & ToolBox : ToolBoxes)
    m_WindowManager->ShowToolBox(ToolBox);

  m_ShownToolBoxes = std::move(ToolBoxes);
}

} // namespace editor
