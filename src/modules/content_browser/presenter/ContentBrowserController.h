#pragma once

#include "ContentItem.h"
#include "IContentBrowserPresenter.h"

#include <cstddef>
#include <functional>
#include <memory>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

class CAssetManager;
class CAssetTypeRegistry;
class CWorkspaceManager;

class CContentBrowserController final:
  public IContentBrowserPresenter
{
public: // Constructor/Destructor

  CContentBrowserController(std::shared_ptr<CAssetManager> _AssetManager, std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<core::CLog> _Log);

  ~CContentBrowserController() override;

  CContentBrowserController(const CContentBrowserController &) = delete;

  CContentBrowserController & operator=(const CContentBrowserController &) = delete;

public: // Interface

  void ApplyPendingActions();

public: // IContentBrowserPresenter state

  const TAssetNode & GetRootFolder() const override;

  const std::filesystem::path & GetCurrentFolder() const noexcept override;

  const std::vector<TContentItem> & GetItems() const noexcept override;

  const std::set<std::filesystem::path> & GetSelection() const noexcept override;

  bool IsSelected(const std::filesystem::path & _Path) const override;

  bool IsDeleteRequested() const noexcept override;

  EContentViewMode GetViewMode() const noexcept override;

public: // IContentBrowserPresenter actions

  void NavigateTo(const std::filesystem::path & _Folder) override;

  void NavigateUp() override;

  void Select(const std::filesystem::path & _Path, ESelectionMode _Mode) override;

  void ClearSelection() override;

  void Open(const std::filesystem::path & _Path) override;

  void CreateFolder() override;

  void Rename(const std::filesystem::path & _Path, const std::string & _NewName) override;

  void DuplicateSelection() override;

  void RequestDeleteSelection() override;

  void ConfirmDelete() override;

  void CancelDelete() override;

  void SetFilter(const std::string & _Filter) override;

  void SetViewMode(EContentViewMode _ViewMode) override;

  void Refresh() override;

private: // Implementation

  void Post(std::function<void()> _Action);

  void ApplyNavigate(const std::filesystem::path & _Folder);

  void ApplySelect(const std::filesystem::path & _Path, ESelectionMode _Mode);

  void ApplyOpen(const std::filesystem::path & _Path);

  void ApplyCreateFolder();

  void ApplyRename(const std::filesystem::path & _Path, const std::string & _NewName);

  void ApplyDuplicateSelection();

  void ApplyDeleteSelection();

  void HandleAssetsChanged();

  void HandleAssetRenamed(const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath);

  void RebuildItems();

  std::string GetTypeName(const TAssetNode & _Node) const;

  void ReportError(const std::string & _Error) const;

private: // Attributes

  std::shared_ptr<CAssetManager>      m_AssetManager;
  std::shared_ptr<CAssetTypeRegistry> m_AssetTypes;
  std::shared_ptr<CWorkspaceManager>  m_WorkspaceManager;
  std::shared_ptr<core::CLog>         m_Log;
  std::filesystem::path               m_CurrentFolder;
  std::vector<TContentItem>           m_Items;
  std::set<std::filesystem::path>     m_Selection;
  std::filesystem::path               m_SelectionAnchor;
  std::string                         m_Filter;
  EContentViewMode                    m_ViewMode;
  bool                                m_IsDeleteRequested;
  bool                                m_IsItemsDirty;
  std::vector<std::function<void()>>  m_PendingActions;
  std::size_t                         m_ChangedConnection;
  std::size_t                         m_RenamedConnection;
};

} // namespace editor
