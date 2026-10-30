#include "ContentBrowserController.h"
#include "core/Log.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetNode.h"
#include "editor/assets/AssetPath.h"
#include "editor/assets/AssetType.h"
#include "editor/assets/AssetTypeInfo.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/workspaces/WorkspaceManager.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace editor
{

namespace
{

std::string ToLower(std::string _Text)
{
  std::transform(_Text.begin(), _Text.end(), _Text.begin(), [](unsigned char _Character)
  {
    return static_cast<char>(std::tolower(_Character));
  });

  return _Text;
}

} // namespace

//
// Constructor/Destructor
//

CContentBrowserController::CContentBrowserController(std::shared_ptr<CAssetManager> _AssetManager, std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<CWorkspaceManager> _WorkspaceManager, std::shared_ptr<core::CLog> _Log):
  m_AssetManager(std::move(_AssetManager)),
  m_AssetTypes(std::move(_AssetTypes)),
  m_WorkspaceManager(std::move(_WorkspaceManager)),
  m_Log(std::move(_Log)),
  m_ViewMode(EContentViewMode::Grid),
  m_IsDeleteRequested(false),
  m_IsItemsDirty(true),
  m_ChangedConnection(0),
  m_RenamedConnection(0)
{
  m_ChangedConnection = m_AssetManager->GetChangedSignal().Connect([this]
  {
    HandleAssetsChanged();
  });

  m_RenamedConnection = m_AssetManager->GetRenamedSignal().Connect([this](const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath)
  {
    HandleAssetRenamed(_OldPath, _NewPath);
  });
}

CContentBrowserController::~CContentBrowserController()
{
  m_AssetManager->GetChangedSignal().Disconnect(m_ChangedConnection);
  m_AssetManager->GetRenamedSignal().Disconnect(m_RenamedConnection);
}

//
// Interface
//

void CContentBrowserController::ApplyPendingActions()
{
  auto Actions = std::move(m_PendingActions);
  m_PendingActions.clear();

  for (const auto & Action : Actions)
    Action();

  if (m_IsItemsDirty)
    RebuildItems();
}

//
// IContentBrowserPresenter state
//

const TAssetNode & CContentBrowserController::GetRootFolder() const
{
  return m_AssetManager->GetRootNode();
}

const std::filesystem::path & CContentBrowserController::GetCurrentFolder() const noexcept
{
  return m_CurrentFolder;
}

const std::vector<TContentItem> & CContentBrowserController::GetItems() const noexcept
{
  return m_Items;
}

const std::set<std::filesystem::path> & CContentBrowserController::GetSelection() const noexcept
{
  return m_Selection;
}

bool CContentBrowserController::IsSelected(const std::filesystem::path & _Path) const
{
  return m_Selection.contains(_Path);
}

bool CContentBrowserController::IsDeleteRequested() const noexcept
{
  return m_IsDeleteRequested;
}

EContentViewMode CContentBrowserController::GetViewMode() const noexcept
{
  return m_ViewMode;
}

//
// IContentBrowserPresenter actions
//

void CContentBrowserController::NavigateTo(const std::filesystem::path & _Folder)
{
  Post([this, _Folder]
  {
    ApplyNavigate(_Folder);
  });
}

void CContentBrowserController::NavigateUp()
{
  Post([this]
  {
    if (!m_CurrentFolder.empty())
      ApplyNavigate(m_CurrentFolder.parent_path());
  });
}

void CContentBrowserController::Select(const std::filesystem::path & _Path, ESelectionMode _Mode)
{
  Post([this, _Path, _Mode]
  {
    ApplySelect(_Path, _Mode);
  });
}

void CContentBrowserController::ClearSelection()
{
  Post([this]
  {
    m_Selection.clear();
    m_SelectionAnchor.clear();
  });
}

void CContentBrowserController::Open(const std::filesystem::path & _Path)
{
  Post([this, _Path]
  {
    ApplyOpen(_Path);
  });
}

void CContentBrowserController::CreateFolder()
{
  Post([this]
  {
    ApplyCreateFolder();
  });
}

void CContentBrowserController::Rename(const std::filesystem::path & _Path, const std::string & _NewName)
{
  Post([this, _Path, _NewName]
  {
    ApplyRename(_Path, _NewName);
  });
}

void CContentBrowserController::DuplicateSelection()
{
  Post([this]
  {
    ApplyDuplicateSelection();
  });
}

void CContentBrowserController::RequestDeleteSelection()
{
  Post([this]
  {
    m_IsDeleteRequested = !m_Selection.empty();
  });
}

void CContentBrowserController::ConfirmDelete()
{
  Post([this]
  {
    ApplyDeleteSelection();
  });
}

void CContentBrowserController::CancelDelete()
{
  Post([this]
  {
    m_IsDeleteRequested = false;
  });
}

void CContentBrowserController::SetFilter(const std::string & _Filter)
{
  Post([this, _Filter]
  {
    m_Filter = ToLower(_Filter);
    RebuildItems();
  });
}

void CContentBrowserController::SetViewMode(EContentViewMode _ViewMode)
{
  Post([this, _ViewMode]
  {
    m_ViewMode = _ViewMode;
  });
}

void CContentBrowserController::Refresh()
{
  Post([this]
  {
    m_AssetManager->Refresh();
  });
}

//
// Implementation
//

void CContentBrowserController::Post(std::function<void()> _Action)
{
  m_PendingActions.push_back(std::move(_Action));
}

void CContentBrowserController::ApplyNavigate(const std::filesystem::path & _Folder)
{
  if (!m_AssetManager->HasNode(_Folder) || !m_AssetManager->GetNode(_Folder).IsDirectory)
    return;

  m_CurrentFolder = _Folder;

  m_Selection.clear();
  m_SelectionAnchor.clear();

  RebuildItems();
}

void CContentBrowserController::ApplySelect(const std::filesystem::path & _Path, ESelectionMode _Mode)
{
  const auto FindItem = [this](const std::filesystem::path & _ItemPath)
  {
    return std::find_if(m_Items.begin(), m_Items.end(), [&_ItemPath](const TContentItem & _Item)
    {
      return _Item.RelativePath == _ItemPath;
    });
  };

  const auto TargetItem = FindItem(_Path);
  if (TargetItem == m_Items.end())
    return;

  const auto AnchorItem = FindItem(m_SelectionAnchor);

  if (_Mode == ESelectionMode::Range && AnchorItem != m_Items.end())
  {
    const auto RangeBegin = std::min(AnchorItem, TargetItem);
    const auto RangeEnd   = std::max(AnchorItem, TargetItem) + 1;

    m_Selection.clear();

    for (auto Item = RangeBegin; Item != RangeEnd; ++Item)
      m_Selection.insert(Item->RelativePath);

    return;
  }

  if (_Mode == ESelectionMode::Toggle)
  {
    if (!m_Selection.erase(_Path))
      m_Selection.insert(_Path);
  }
  else
  {
    m_Selection = {_Path};
  }

  m_SelectionAnchor = _Path;
}

void CContentBrowserController::ApplyOpen(const std::filesystem::path & _Path)
{
  if (!m_AssetManager->HasNode(_Path))
    return;

  const TAssetNode & Node = m_AssetManager->GetNode(_Path);

  if (Node.IsDirectory)
    ApplyNavigate(_Path);
  else
    m_WorkspaceManager->OpenAsset(Node.Record);
}

void CContentBrowserController::ApplyCreateFolder()
{
  std::filesystem::path CreatedPath;
  std::string           Error;

  if (!m_AssetManager->CreateFolder(m_CurrentFolder, CreatedPath, Error))
  {
    ReportError(Error);
    return;
  }

  m_Selection       = {CreatedPath};
  m_SelectionAnchor = CreatedPath;
}

void CContentBrowserController::ApplyRename(const std::filesystem::path & _Path, const std::string & _NewName)
{
  std::filesystem::path RenamedPath;
  std::string           Error;

  if (!m_AssetManager->Rename(_Path, _NewName, RenamedPath, Error))
  {
    ReportError(Error);
    return;
  }

  m_Selection       = {RenamedPath};
  m_SelectionAnchor = RenamedPath;
}

void CContentBrowserController::ApplyDuplicateSelection()
{
  std::set<std::filesystem::path> CreatedPaths;

  for (const auto & Path : m_Selection)
  {
    std::filesystem::path CreatedPath;
    std::string           Error;

    if (m_AssetManager->Duplicate(Path, CreatedPath, Error))
      CreatedPaths.insert(CreatedPath);
    else
      ReportError(Error);
  }

  if (CreatedPaths.empty())
    return;

  m_Selection       = std::move(CreatedPaths);
  m_SelectionAnchor = *m_Selection.begin();
}

void CContentBrowserController::ApplyDeleteSelection()
{
  m_IsDeleteRequested = false;

  const auto PathsToDelete = m_Selection;

  for (const auto & Path : PathsToDelete)
  {
    if (!m_AssetManager->HasNode(Path))
      continue;

    std::string Error;
    if (!m_AssetManager->Remove(Path, Error))
      ReportError(Error);
  }

  m_Selection.clear();
  m_SelectionAnchor.clear();
}

void CContentBrowserController::HandleAssetsChanged()
{
  while (!m_CurrentFolder.empty() && !m_AssetManager->HasNode(m_CurrentFolder))
    m_CurrentFolder = m_CurrentFolder.parent_path();

  std::erase_if(m_Selection, [this](const std::filesystem::path & _Path)
  {
    return !m_AssetManager->HasNode(_Path);
  });

  RebuildItems();
}

void CContentBrowserController::HandleAssetRenamed(const std::filesystem::path & _OldPath, const std::filesystem::path & _NewPath)
{
  if (IsSameOrInside(m_CurrentFolder, _OldPath))
    m_CurrentFolder = RebasePath(m_CurrentFolder, _OldPath, _NewPath);

  if (IsSameOrInside(m_SelectionAnchor, _OldPath))
    m_SelectionAnchor = RebasePath(m_SelectionAnchor, _OldPath, _NewPath);

  std::set<std::filesystem::path> Selection;

  for (const auto & Path : m_Selection)
    Selection.insert(IsSameOrInside(Path, _OldPath) ? RebasePath(Path, _OldPath, _NewPath) : Path);

  m_Selection = std::move(Selection);
}

void CContentBrowserController::RebuildItems()
{
  m_IsItemsDirty = false;
  m_Items.clear();

  if (!m_AssetManager->HasNode(m_CurrentFolder))
    return;

  for (const auto & Child : m_AssetManager->GetNode(m_CurrentFolder).Children)
  {
    if (!m_Filter.empty() && ToLower(Child.Name).find(m_Filter) == std::string::npos)
      continue;

    TContentItem Item;
    Item.RelativePath = Child.Record.RelativePath;
    Item.Name         = Child.Name;
    Item.TypeName     = GetTypeName(Child);
    Item.SizeBytes    = Child.Record.SizeBytes;
    Item.IsDirectory  = Child.IsDirectory;

    m_Items.push_back(std::move(Item));
  }

  std::sort(m_Items.begin(), m_Items.end(), [](const TContentItem & _Left, const TContentItem & _Right)
  {
    if (_Left.IsDirectory != _Right.IsDirectory)
      return _Left.IsDirectory;

    return ToLower(_Left.Name) < ToLower(_Right.Name);
  });
}

std::string CContentBrowserController::GetTypeName(const TAssetNode & _Node) const
{
  if (_Node.IsDirectory)
    return "Folder";

  if (const auto AssetType = m_AssetTypes->FindByExtension(_Node.Record.RelativePath.extension().string()))
    return AssetType->GetInfo().Name;

  return "File";
}

void CContentBrowserController::ReportError(const std::string & _Error) const
{
  m_Log->Write(core::ELogLevel::Error, _Error);
}

} // namespace editor
