#pragma once

#include "EditorWorkspace.h"
#include "core/EditorService.h"
#include "core/Signal.h"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

struct TAssetRecord;
class CAssetTypeRegistry;
class IAttachedEditor;

class CWorkspaceManager
{
public: // Types

  using TActiveChangedSignal   = core::CSignal<>;
  using TAttachedChangedSignal = core::CSignal<>;

public: // Constructor/Destructor

  CWorkspaceManager(std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<core::CLog> _Log);

  ~CWorkspaceManager();

public: // Interface

  bool OpenAsset(const TAssetRecord & _Asset);

  bool AddWorkspace(std::unique_ptr<CEditorWorkspace> _Workspace);

  void SetActive(std::size_t _Index);

  void Close(std::size_t _Index);

  void CloseActive();

  void CloseAll();

  void CloseAssets(const std::filesystem::path & _Path);

  void RelocateAssets(const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath, const std::filesystem::path & _AssetsRoot);

  std::optional<std::size_t> FindWorkspace(const std::filesystem::path & _AbsolutePath) const;

  bool IsAssetOpen(const std::filesystem::path & _AbsolutePath) const;

  template <class THost>
  std::optional<std::size_t> FindHost() const;

  std::shared_ptr<IAttachedEditor> FindAttachedEditor(std::size_t _WorkspaceIndex, const std::filesystem::path & _AbsolutePath) const;

  bool AttachEditor(std::size_t _WorkspaceIndex, std::shared_ptr<IAttachedEditor> _Editor);

  void DetachEditor(std::size_t _WorkspaceIndex, const IAttachedEditor & _Editor);

  bool HasDirtyWorkspaces() const;

  bool SaveAll(std::string & _Error);

  void Tick(float _DeltaTime);

  bool HasActive() const noexcept;

  std::size_t GetActiveIndex() const noexcept;

  CEditorWorkspace * GetActive() const noexcept;

  const std::vector<std::unique_ptr<CEditorWorkspace>> & GetWorkspaces() const noexcept;

public: // Signals

  TActiveChangedSignal & GetActiveChangedSignal() noexcept;

  TAttachedChangedSignal & GetAttachedChangedSignal() noexcept;

private: // Implementation

  std::optional<std::size_t> FindWorkspaceIf(const std::function<bool(const CEditorWorkspace &)> & _Predicate) const;

  void CloseAttachedAssets(CEditorWorkspace & _Workspace, const std::filesystem::path & _Path);

  void RebuildPathIndex();

private: // Attributes

  std::shared_ptr<CAssetTypeRegistry>            m_AssetTypes;
  std::shared_ptr<core::CLog>                    m_Log;
  std::vector<std::unique_ptr<CEditorWorkspace>> m_Workspaces;
  std::unordered_map<std::string, std::size_t>   m_IndexByPath;
  std::size_t                                    m_ActiveIndex;
  bool                                           m_HasActive;
  TActiveChangedSignal                           m_ActiveChangedSignal;
  TAttachedChangedSignal                         m_AttachedChangedSignal;
};

//
// Template implementation
//

template <class THost>
std::optional<std::size_t> CWorkspaceManager::FindHost() const
{
  return FindWorkspaceIf([](const CEditorWorkspace & _Workspace)
  {
    return _Workspace.GetServices().HasService<THost>();
  });
}

} // namespace editor
