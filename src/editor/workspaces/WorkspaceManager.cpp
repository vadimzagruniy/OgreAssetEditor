#include "WorkspaceManager.h"
#include "AssetInstance.h"
#include "EditorWorkspace.h"
#include "core/Log.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetPath.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetType.h"
#include "editor/assets/AssetTypeRegistry.h"

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
  return MakePathKey(_Workspace.GetInstance()->GetRecord().AbsolutePath);
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

  if (!Workspace)
  {
    m_Log->Write(core::ELogLevel::Error, Error.empty() ? "Asset open failed" : Error);
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
    m_Workspaces[m_ActiveIndex]->OnDeactivated();

  m_ActiveIndex = _Index;
  m_HasActive   = true;

  m_Workspaces[m_ActiveIndex]->OnActivated();

  m_ActiveChangedSignal.Emit();
}

void CWorkspaceManager::Close(std::size_t _Index)
{
  if (_Index >= m_Workspaces.size())
    return;

  const bool WasActive = m_HasActive && _Index == m_ActiveIndex;
  if (WasActive)
  {
    m_Workspaces[_Index]->OnDeactivated();
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
    m_Workspaces[m_ActiveIndex]->OnDeactivated();

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
    const auto & Record = m_Workspaces[WorkspaceIndex - 1]->GetInstance()->GetRecord();

    if (IsSameOrInside(Record.RelativePath, _Path))
      Close(WorkspaceIndex - 1);
  }
}

void CWorkspaceManager::RelocateAssets(const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath, const std::filesystem::path & _AssetsRoot)
{
  for (const auto & Workspace : m_Workspaces)
  {
    auto & Instance = *Workspace->GetInstance();

    if (!IsSameOrInside(Instance.GetRecord().RelativePath, _OldPath))
      continue;

    TAssetRecord Record = Instance.GetRecord();
    Record.RelativePath = RebasePath(Record.RelativePath, _OldPath, _NewPath);
    Record.AbsolutePath = (_AssetsRoot / Record.RelativePath).lexically_normal();

    Instance.SetRecord(std::move(Record));
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

//
// Implementation
//

void CWorkspaceManager::RebuildPathIndex()
{
  m_IndexByPath.clear();

  for (std::size_t WorkspaceIndex = 0; WorkspaceIndex < m_Workspaces.size(); ++WorkspaceIndex)
    m_IndexByPath.emplace(MakePathKey(*m_Workspaces[WorkspaceIndex]), WorkspaceIndex);
}

} // namespace editor
