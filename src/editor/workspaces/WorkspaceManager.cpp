#include "WorkspaceManager.h"
#include "EditorWorkspace.h"
#include "IAttachedEditor.h"
#include "core/Log.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetPath.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetType.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/documents/AssetDocument.h"

#include <algorithm>
#include <filesystem>
#include <utility>

namespace editor
{

namespace
{

std::string MakePathKey(const std::filesystem::path & _Path)
{
  return std::filesystem::weakly_canonical(_Path).generic_string();
}

std::string MakePathKey(const CEditorWorkspace & _Workspace)
{
  return MakePathKey(_Workspace.GetDocument().GetRecord().AbsolutePath);
}

void RelocateDocument(CAssetDocument & _Document, const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath, const std::filesystem::path & _AssetsRoot)
{
  if (!IsSameOrInside(_Document.GetRecord().RelativePath, _OldPath))
    return;

  TAssetRecord Record = _Document.GetRecord();
  Record.RelativePath = RebasePath(Record.RelativePath, _OldPath, _NewPath);
  Record.AbsolutePath = (_AssetsRoot / Record.RelativePath).lexically_normal();

  _Document.SetRecord(std::move(Record));
}

} // namespace

//
// Constructor/Destructor
//

CWorkspaceManager::CWorkspaceManager(std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<core::CLog> _Log):
  m_AssetTypes(std::move(_AssetTypes)),
  m_Log(std::move(_Log)),
  m_ActiveIndex(0),
  m_HasActive(false)
{
  // Empty
}

CWorkspaceManager::~CWorkspaceManager() = default;

//
// Interface
//

bool CWorkspaceManager::OpenAsset(const TAssetRecord & _Asset)
{
  // Already opened asset is just activated, without loading it again.
  if (const auto OpenedIterator = m_IndexByPath.find(MakePathKey(_Asset.AbsolutePath)); OpenedIterator != m_IndexByPath.end())
  {
    SetActive(OpenedIterator->second);
    return true;
  }

  const auto AssetType = m_AssetTypes->FindByExtension(_Asset.RelativePath.extension().string());
  if (!AssetType)
  {
    m_Log->Write(core::ELogLevel::Warning, "No asset module for " + _Asset.RelativePath.string());
    return false;
  }

  std::string Error;
  auto        Workspace = AssetType->OpenAsset(_Asset, CAssetManager::ResourceGroup, Error);

  if (!Workspace && Error.empty())
    return true;

  if (!Workspace)
  {
    m_Log->Write(core::ELogLevel::Error, Error);
    return false;
  }

  return AddWorkspace(std::move(Workspace));
}

bool CWorkspaceManager::AddWorkspace(std::unique_ptr<CEditorWorkspace> _Workspace)
{
  if (!_Workspace)
    return false;

  const auto PathKey = MakePathKey(*_Workspace);

  if (const auto OpenedIterator = m_IndexByPath.find(PathKey); OpenedIterator != m_IndexByPath.end())
  {
    SetActive(OpenedIterator->second);
    return false;
  }

  m_IndexByPath.emplace(PathKey, m_Workspaces.size());
  m_Workspaces.push_back(std::move(_Workspace));

  SetActive(m_Workspaces.size() - 1);

  return true;
}

void CWorkspaceManager::SetActive(std::size_t _Index)
{
  if (m_HasActive && m_ActiveIndex == _Index)
    return;

  if (m_HasActive)
    m_Workspaces[m_ActiveIndex]->Deactivate();

  m_ActiveIndex = _Index;
  m_HasActive   = true;

  m_Workspaces[m_ActiveIndex]->Activate();

  m_ActiveChangedSignal.Emit();
}

void CWorkspaceManager::Close(std::size_t _Index)
{
  if (_Index >= m_Workspaces.size())
    return;

  const bool WasActive = m_HasActive && _Index == m_ActiveIndex;
  if (WasActive)
  {
    m_Workspaces[_Index]->Deactivate();
    m_HasActive = false;
  }

  m_Workspaces.erase(m_Workspaces.begin() + static_cast<std::ptrdiff_t>(_Index));
  RebuildPathIndex();

  if (m_Workspaces.empty())
  {
    m_HasActive   = false;
    m_ActiveIndex = 0;

    m_ActiveChangedSignal.Emit();
    return;
  }

  if (WasActive)
    SetActive(std::min(_Index, m_Workspaces.size() - 1));
  else
  if (_Index < m_ActiveIndex)
    --m_ActiveIndex;
}

void CWorkspaceManager::CloseActive()
{
  if (m_HasActive)
    Close(m_ActiveIndex);
}

void CWorkspaceManager::CloseAll()
{
  if (m_HasActive)
    m_Workspaces[m_ActiveIndex]->Deactivate();

  m_Workspaces.clear();
  m_IndexByPath.clear();

  m_HasActive   = false;
  m_ActiveIndex = 0;

  m_ActiveChangedSignal.Emit();
}

void CWorkspaceManager::CloseAssets(const std::filesystem::path & _Path)
{
  for (std::size_t WorkspaceIndex = m_Workspaces.size(); WorkspaceIndex > 0; --WorkspaceIndex)
  {
    auto &       Workspace = *m_Workspaces[WorkspaceIndex - 1];
    const auto & Record    = Workspace.GetDocument().GetRecord();

    if (IsSameOrInside(Record.RelativePath, _Path))
      Close(WorkspaceIndex - 1);
    else
      CloseAttachedAssets(Workspace, _Path);
  }
}

void CWorkspaceManager::RelocateAssets(const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath, const std::filesystem::path & _AssetsRoot)
{
  for (const auto & Workspace : m_Workspaces)
  {
    RelocateDocument(Workspace->GetDocument(), _OldPath, _NewPath, _AssetsRoot);

    for (const auto & Editor : Workspace->GetAttachedEditors())
      RelocateDocument(Editor->GetDocument(), _OldPath, _NewPath, _AssetsRoot);
  }

  RebuildPathIndex();
}

std::optional<std::size_t> CWorkspaceManager::FindWorkspace(const std::filesystem::path & _AbsolutePath) const
{
  const auto WorkspaceIterator = m_IndexByPath.find(MakePathKey(_AbsolutePath));
  if (WorkspaceIterator == m_IndexByPath.end())
    return std::nullopt;

  return WorkspaceIterator->second;
}

std::shared_ptr<IAttachedEditor> CWorkspaceManager::FindAttachedEditor(std::size_t _WorkspaceIndex, const std::filesystem::path & _AbsolutePath) const
{
  if (_WorkspaceIndex >= m_Workspaces.size())
    return nullptr;

  const auto PathKey = MakePathKey(_AbsolutePath);

  for (const auto & Editor : m_Workspaces[_WorkspaceIndex]->GetAttachedEditors())
  {
    if (MakePathKey(Editor->GetDocument().GetRecord().AbsolutePath) == PathKey)
      return Editor;
  }

  return nullptr;
}

bool CWorkspaceManager::AttachEditor(std::size_t _WorkspaceIndex, std::shared_ptr<IAttachedEditor> _Editor)
{
  if (!_Editor || _WorkspaceIndex >= m_Workspaces.size())
    return false;

  if (FindAttachedEditor(_WorkspaceIndex, _Editor->GetDocument().GetRecord().AbsolutePath))
    return false;

  m_Workspaces[_WorkspaceIndex]->AttachEditor(std::move(_Editor));
  m_AttachedChangedSignal.Emit();

  return true;
}

void CWorkspaceManager::DetachEditor(std::size_t _WorkspaceIndex, const IAttachedEditor & _Editor)
{
  if (_WorkspaceIndex >= m_Workspaces.size())
    return;

  m_Workspaces[_WorkspaceIndex]->DetachEditor(_Editor);
  m_AttachedChangedSignal.Emit();
}

bool CWorkspaceManager::HasDirtyWorkspaces() const
{
  return std::any_of(m_Workspaces.begin(), m_Workspaces.end(), [](const std::unique_ptr<CEditorWorkspace> & _Workspace)
  {
    return _Workspace->IsDirty();
  });
}

bool CWorkspaceManager::SaveAll(std::string & _Error)
{
  bool IsSaved = true;

  for (const auto & Workspace : m_Workspaces)
  {
    if (!Workspace->IsDirty())
      continue;

    std::string Error;
    if (!Workspace->Save(Error))
    {
      _Error += Error.empty() ? "Cannot save " + Workspace->GetTitle() + "\n" : Error + "\n";
      IsSaved = false;
    }
  }

  return IsSaved;
}

void CWorkspaceManager::Tick(float _DeltaTime)
{
  for (const auto & Workspace : m_Workspaces)
    Workspace->Tick(_DeltaTime);
}

bool CWorkspaceManager::HasActive() const noexcept
{
  return m_HasActive;
}

std::size_t CWorkspaceManager::GetActiveIndex() const noexcept
{
  return m_ActiveIndex;
}

CEditorWorkspace * CWorkspaceManager::GetActive() const noexcept
{
  return m_HasActive ? m_Workspaces[m_ActiveIndex].get() : nullptr;
}

const std::vector<std::unique_ptr<CEditorWorkspace>> & CWorkspaceManager::GetWorkspaces() const noexcept
{
  return m_Workspaces;
}

//
// Signals
//

CWorkspaceManager::TActiveChangedSignal & CWorkspaceManager::GetActiveChangedSignal() noexcept
{
  return m_ActiveChangedSignal;
}

CWorkspaceManager::TAttachedChangedSignal & CWorkspaceManager::GetAttachedChangedSignal() noexcept
{
  return m_AttachedChangedSignal;
}

//
// Implementation
//

std::optional<std::size_t> CWorkspaceManager::FindWorkspaceIf(const std::function<bool(const CEditorWorkspace &)> & _Predicate) const
{
  if (m_HasActive && _Predicate(*m_Workspaces[m_ActiveIndex]))
    return m_ActiveIndex;

  for (std::size_t WorkspaceIndex = 0; WorkspaceIndex < m_Workspaces.size(); ++WorkspaceIndex)
  {
    if (_Predicate(*m_Workspaces[WorkspaceIndex]))
      return WorkspaceIndex;
  }

  return std::nullopt;
}

void CWorkspaceManager::CloseAttachedAssets(CEditorWorkspace & _Workspace, const std::filesystem::path & _Path)
{
  std::vector<std::shared_ptr<IAttachedEditor>> ClosedEditors;

  for (const auto & Editor : _Workspace.GetAttachedEditors())
  {
    if (IsSameOrInside(Editor->GetDocument().GetRecord().RelativePath, _Path))
      ClosedEditors.push_back(Editor);
  }

  if (ClosedEditors.empty())
    return;

  for (const auto & Editor : ClosedEditors)
    _Workspace.DetachEditor(*Editor);

  m_AttachedChangedSignal.Emit();
}

void CWorkspaceManager::RebuildPathIndex()
{
  m_IndexByPath.clear();

  for (std::size_t WorkspaceIndex = 0; WorkspaceIndex < m_Workspaces.size(); ++WorkspaceIndex)
    m_IndexByPath.emplace(MakePathKey(*m_Workspaces[WorkspaceIndex]), WorkspaceIndex);
}

} // namespace editor
