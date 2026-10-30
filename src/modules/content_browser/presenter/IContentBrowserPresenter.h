#pragma once

#include "ContentViewMode.h"
#include "SelectionMode.h"

#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace editor
{

struct TAssetNode;
struct TContentItem;

class IContentBrowserPresenter
{
public: // Destructor

  virtual ~IContentBrowserPresenter();

public: // State

  virtual const TAssetNode & GetRootFolder() const = 0;

  virtual const std::filesystem::path & GetCurrentFolder() const noexcept = 0;

  virtual const std::vector<TContentItem> & GetItems() const noexcept = 0;

  virtual const std::set<std::filesystem::path> & GetSelection() const noexcept = 0;

  virtual bool IsSelected(const std::filesystem::path & _Path) const = 0;

  virtual bool IsDeleteRequested() const noexcept = 0;

  virtual EContentViewMode GetViewMode() const noexcept = 0;

public: // Actions

  virtual void NavigateTo(const std::filesystem::path & _Folder) = 0;

  virtual void NavigateUp() = 0;

  virtual void Select(const std::filesystem::path & _Path, ESelectionMode _Mode) = 0;

  virtual void ClearSelection() = 0;

  virtual void Open(const std::filesystem::path & _Path) = 0;

  virtual void CreateFolder() = 0;

  virtual void Rename(const std::filesystem::path & _Path, const std::string & _NewName) = 0;

  virtual void DuplicateSelection() = 0;

  virtual void RequestDeleteSelection() = 0;

  virtual void ConfirmDelete() = 0;

  virtual void CancelDelete() = 0;

  virtual void SetFilter(const std::string & _Filter) = 0;

  virtual void SetViewMode(EContentViewMode _ViewMode) = 0;

  virtual void Refresh() = 0;
};

} // namespace editor
