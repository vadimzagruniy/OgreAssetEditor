#pragma once

#include "ContentItem.h"
#include "IContentBrowserPresenter.h"
#include "editor/assets/AssetId.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace editor
{

namespace core
{
class CActionQueue;
class CLog;
} // namespace core

class CAssetDatabase;
class CAssetDependencyIndex;
class CAssetFactoryRegistry;
class CAssetManager;
class CAssetTypeRegistry;
class CWorkspaceManager;

class CContentBrowserController final:
  public IContentBrowserPresenter
{
public: // Constructor/Destructor

  CContentBrowserController(std::shared_ptr<CAssetManager> _AssetManager, std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<CAssetFactoryRegistry> _AssetFactories, std::shared_ptr<const CAssetDatabase> _Assets, std::shared_ptr<CAssetDependencyIndex> _Dependencies, std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<core::CLog> _Log, std::shared_ptr<core::CActionQueue> _ActionQueue);

  ~CContentBrowserController() override;

  CContentBrowserController(const CContentBrowserController &) = delete;

  CContentBrowserController & operator=(const CContentBrowserController &) = delete;

public: // IContentBrowserPresenter state

  const TAssetNode & GetRootFolder() const override;

  const std::filesystem::path & GetCurrentFolder() const noexcept override;

  const std::vector<TContentItem> & GetItems() const noexcept override;

  const std::set<std::filesystem::path> & GetSelection() const noexcept override;

  bool IsSelected(const std::filesystem::path & _Path) const override;

  bool IsDeleteRequested() const noexcept override;

  const std::vector<std::string> & GetDeleteReferrers() const noexcept override;

  const std::optional<TAssetReferencesReport> & GetReferencesReport() const noexcept override;

  const std::optional<TAssetCreationRequest> & GetCreationRequest() const noexcept override;

  bool CheckAssetName(const std::filesystem::path & _Folder, const std::string & _Name, std::string & _Error) const override;

  EContentViewMode GetViewMode() const noexcept override;

  std::vector<TAssetMenuEntry> GetContextActions(const std::filesystem::path & _Path) const override;

  std::vector<TAssetFactoryInfo> GetAssetFactories() const override;

public: // IContentBrowserPresenter actions

  void NavigateTo(const std::filesystem::path & _Folder) override;

  void NavigateUp() override;

  void Select(const std::filesystem::path & _Path, ESelectionMode _Mode) override;

  void ClearSelection() override;

  void Open(const std::filesystem::path & _Path) override;

  void CreateFolder() override;

  void RequestCreateAsset(const std::string & _FactoryIdentifier) override;

  void ConfirmCreateAsset(const std::filesystem::path & _Folder, const std::string & _Name) override;

  void CancelCreateAsset() override;

  void Rename(const std::filesystem::path & _Path, const std::string & _NewName) override;

  void DuplicateSelection() override;

  void RequestDeleteSelection() override;

  void ConfirmDelete() override;

  void CancelDelete() override;

  void FindReferences(const std::filesystem::path & _Path) override;

  void CloseReferencesReport() override;

  void SetFilter(const std::string & _Filter) override;

  void SetViewMode(EContentViewMode _ViewMode) override;

  void Refresh() override;

private: // Implementation

  void Post(std::function<void()> _Action);

  void ApplyNavigate(const std::filesystem::path & _Folder);

  void ApplySelect(const std::filesystem::path & _Path, ESelectionMode _Mode);

  void ApplyOpen(const std::filesystem::path & _Path);

  void ApplyCreateFolder();

  void ApplyRequestCreateAsset(const std::string & _FactoryIdentifier);

  void ApplyCreateAsset(const std::filesystem::path & _Folder, const std::string & _Name);

  void ApplyRename(const std::filesystem::path & _Path, const std::string & _NewName);

  void ApplyDuplicateSelection();

  void ApplyDeleteSelection();

  void ApplyFindReferences(const std::filesystem::path & _Path);

  void ApplyRevealInExplorer(const std::filesystem::path & _Path) const;

  void ApplyCopyPaths(const std::set<std::filesystem::path> & _Paths) const;

  std::set<std::filesystem::path> GetActionTargets(const std::filesystem::path & _Path) const;

  void AddAssetTypeActions(const std::filesystem::path & _Path, std::vector<TAssetMenuEntry> & _Entries) const;

  std::vector<std::string> CollectExternalReferrers(const std::set<std::filesystem::path> & _Paths) const;

  void CollectAssetIds(const TAssetNode & _Node, std::set<TAssetId> & _AssetIds) const;

  void HandleAssetsChanged();

  void HandleAssetRenamed(const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath);

  void RebuildItems();

  std::string GetTypeName(const TAssetNode & _Node) const;

  void ReportError(const std::string & _Error) const;

private: // Attributes

  std::shared_ptr<CAssetManager>         m_AssetManager;
  std::shared_ptr<CAssetTypeRegistry>    m_AssetTypes;
  std::shared_ptr<CAssetFactoryRegistry> m_AssetFactories;
  std::shared_ptr<const CAssetDatabase>  m_Assets;
  std::shared_ptr<CAssetDependencyIndex> m_Dependencies;
  std::shared_ptr<CWorkspaceManager>     m_WorkspaceManager;
  std::shared_ptr<core::CLog>            m_Log;
  std::shared_ptr<core::CActionQueue>    m_ActionQueue;
  std::filesystem::path                  m_CurrentFolder;
  std::vector<TContentItem>              m_Items;
  std::set<std::filesystem::path>        m_Selection;
  std::filesystem::path                  m_SelectionAnchor;
  std::string                            m_Filter;
  EContentViewMode                       m_ViewMode;
  bool                                   m_IsDeleteRequested;
  std::vector<std::string>               m_DeleteReferrers;
  std::optional<TAssetReferencesReport>  m_ReferencesReport;
  std::optional<TAssetCreationRequest>   m_CreationRequest;
  std::size_t                            m_ChangedConnection;
  std::size_t                            m_RenamedConnection;
};

} // namespace editor
