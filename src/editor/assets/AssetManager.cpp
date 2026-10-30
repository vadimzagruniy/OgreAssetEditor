#include "AssetManager.h"

#include <OgreResourceGroupManager.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <system_error>
#include <utility>

namespace editor
{

namespace
{

constexpr const char * NewFolderName = "New Folder";
constexpr const char * CopySuffix    = " Copy";

std::string ToLower(std::string _Text)
{
  std::transform(_Text.begin(), _Text.end(), _Text.begin(), [](unsigned char _Character)
  {
    return static_cast<char>(std::tolower(_Character));
  });

  return _Text;
}

bool IsNodeOrderedBefore(const TAssetNode & _Left, const TAssetNode & _Right)
{
  if (_Left.IsDirectory != _Right.IsDirectory)
    return _Left.IsDirectory;

  return ToLower(_Left.Name) < ToLower(_Right.Name);
}

bool IsValidName(const std::string & _Name)
{
  if (_Name.empty() || _Name == "." || _Name == "..")
    return false;

  return _Name.find_first_of("\\/:*?\"<>|") == std::string::npos;
}

std::filesystem::path MakeUniquePath(const std::filesystem::path & _Folder, const std::string & _Stem, const std::string & _Extension)
{
  std::error_code       ErrorCode;
  std::filesystem::path Candidate = _Folder / (_Stem + _Extension);

  for (int Index = 2; std::filesystem::exists(Candidate, ErrorCode); ++Index)
    Candidate = _Folder / (_Stem + " " + std::to_string(Index) + _Extension);

  return Candidate;
}

} // namespace

//
// Constructor/Destructor
//

CAssetManager::CAssetManager():
  m_IsResourcesMounted(false)
{
  // Empty
}

CAssetManager::~CAssetManager() = default;

//
// Interface
//

bool CAssetManager::OpenProject(const std::filesystem::path & _Root)
{
  std::error_code ErrorCode;

  m_ProjectRoot = std::filesystem::absolute(_Root, ErrorCode).lexically_normal();
  if (ErrorCode)
  {
    m_LastError = "Cannot resolve project directory: " + ErrorCode.message();
    return false;
  }

  m_AssetsRoot = m_ProjectRoot / "Assets";
  if (!std::filesystem::is_directory(m_AssetsRoot, ErrorCode))
  {
    m_LastError = ErrorCode ? ErrorCode.message() : "Project must contain an Assets directory.";
    return false;
  }

  return Refresh();
}

bool CAssetManager::Refresh()
{
  m_LastError.clear();

  std::error_code ErrorCode;
  if (!std::filesystem::is_directory(m_AssetsRoot, ErrorCode))
  {
    m_LastError = "Assets directory does not exist.";
    return false;
  }

  m_RootNode                     = TAssetNode();
  m_RootNode.Name                = "Assets";
  m_RootNode.IsDirectory         = true;
  m_RootNode.Record.AbsolutePath = m_AssetsRoot;

  ScanFolder(m_RootNode);

  if (m_IsResourcesMounted)
    RemountResources();

  m_ChangedSignal.Emit();

  return m_LastError.empty();
}

bool CAssetManager::MountResources()
{
  if (m_AssetsRoot.empty() || !std::filesystem::is_directory(m_AssetsRoot))
    return false;

  auto & ResourceGroups = Ogre::ResourceGroupManager::getSingleton();
  if (!ResourceGroups.resourceGroupExists(ResourceGroup))
    ResourceGroups.createResourceGroup(ResourceGroup);

  ResourceGroups.addResourceLocation(m_AssetsRoot.string(), "FileSystem", ResourceGroup, true, true);

  m_IsResourcesMounted = true;

  return true;
}

//
// Operations
//

bool CAssetManager::CreateFolder(const std::filesystem::path & _ParentFolder, std::filesystem::path & _CreatedPath, std::string & _Error)
{
  if (!HasNode(_ParentFolder) || !GetNode(_ParentFolder).IsDirectory)
  {
    _Error = "Folder does not exist: " + _ParentFolder.generic_string();
    return false;
  }

  const std::filesystem::path AbsolutePath = MakeUniquePath(ToAbsolute(_ParentFolder), NewFolderName, "");

  std::error_code ErrorCode;
  if (!std::filesystem::create_directory(AbsolutePath, ErrorCode))
  {
    _Error = "Cannot create folder: " + (ErrorCode ? ErrorCode.message() : AbsolutePath.string());
    return false;
  }

  _CreatedPath = _ParentFolder / AbsolutePath.filename();

  Refresh();

  return true;
}

bool CAssetManager::Rename(const std::filesystem::path & _Path, const std::string & _NewName, std::filesystem::path & _RenamedPath, std::string & _Error)
{
  if (_Path.empty() || !HasNode(_Path))
  {
    _Error = "Cannot rename: " + _Path.generic_string();
    return false;
  }

  if (!IsValidName(_NewName))
  {
    _Error = "Invalid name: " + _NewName;
    return false;
  }

  const std::filesystem::path NewPath = _Path.parent_path() / _NewName;
  if (NewPath == _Path)
  {
    _RenamedPath = _Path;
    return true;
  }

  std::error_code ErrorCode;
  if (std::filesystem::exists(ToAbsolute(NewPath), ErrorCode))
  {
    _Error = "Already exists: " + NewPath.generic_string();
    return false;
  }

  std::filesystem::rename(ToAbsolute(_Path), ToAbsolute(NewPath), ErrorCode);
  if (ErrorCode)
  {
    _Error = "Cannot rename " + _Path.generic_string() + ": " + ErrorCode.message();
    return false;
  }

  _RenamedPath = NewPath;

  m_RenamedSignal.Emit(_Path, NewPath);
  Refresh();

  return true;
}

bool CAssetManager::Duplicate(const std::filesystem::path & _Path, std::filesystem::path & _CreatedPath, std::string & _Error)
{
  if (_Path.empty() || !HasNode(_Path))
  {
    _Error = "Cannot duplicate: " + _Path.generic_string();
    return false;
  }

  const std::filesystem::path SourcePath = ToAbsolute(_Path);
  const std::string           Stem       = SourcePath.stem().string() + CopySuffix;
  const std::filesystem::path TargetPath = MakeUniquePath(SourcePath.parent_path(), Stem, SourcePath.extension().string());

  std::error_code ErrorCode;
  std::filesystem::copy(SourcePath, TargetPath, std::filesystem::copy_options::recursive, ErrorCode);

  if (ErrorCode)
  {
    _Error = "Cannot duplicate " + _Path.generic_string() + ": " + ErrorCode.message();
    return false;
  }

  _CreatedPath = _Path.parent_path() / TargetPath.filename();

  Refresh();

  return true;
}

bool CAssetManager::Remove(const std::filesystem::path & _Path, std::string & _Error)
{
  if (_Path.empty() || !HasNode(_Path))
  {
    _Error = "Cannot delete: " + _Path.generic_string();
    return false;
  }

  std::error_code ErrorCode;
  std::filesystem::remove_all(ToAbsolute(_Path), ErrorCode);

  if (ErrorCode)
  {
    _Error = "Cannot delete " + _Path.generic_string() + ": " + ErrorCode.message();
    Refresh();
    return false;
  }

  m_RemovedSignal.Emit(_Path);
  Refresh();

  return true;
}

//
// Accessors
//

const std::filesystem::path & CAssetManager::GetProjectRoot() const noexcept
{
  return m_ProjectRoot;
}

const std::filesystem::path & CAssetManager::GetAssetsRoot() const noexcept
{
  return m_AssetsRoot;
}

const TAssetNode & CAssetManager::GetRootNode() const noexcept
{
  return m_RootNode;
}

bool CAssetManager::HasNode(const std::filesystem::path & _RelativePath) const
{
  return FindNode(_RelativePath).has_value();
}

const TAssetNode & CAssetManager::GetNode(const std::filesystem::path & _RelativePath) const
{
  const auto Node = FindNode(_RelativePath);
  assert(Node && "Asset node does not exist");

  return Node->get();
}

const std::string & CAssetManager::GetLastError() const noexcept
{
  return m_LastError;
}

//
// Signals
//

CAssetManager::TChangedSignal & CAssetManager::GetChangedSignal() noexcept
{
  return m_ChangedSignal;
}

CAssetManager::TRemovedSignal & CAssetManager::GetRemovedSignal() noexcept
{
  return m_RemovedSignal;
}

CAssetManager::TRenamedSignal & CAssetManager::GetRenamedSignal() noexcept
{
  return m_RenamedSignal;
}

//
// Implementation
//

std::optional<std::reference_wrapper<const TAssetNode>> CAssetManager::FindNode(const std::filesystem::path & _RelativePath) const
{
  std::reference_wrapper<const TAssetNode> Current = m_RootNode;

  for (const auto & Element : _RelativePath)
  {
    const auto & Children = Current.get().Children;
    const auto   Child    = std::find_if(Children.begin(), Children.end(), [&Element](const TAssetNode & _Child)
    {
      return _Child.Name == Element.string();
    });

    if (Child == Children.end())
      return std::nullopt;

    Current = *Child;
  }

  return Current;
}

void CAssetManager::ScanFolder(TAssetNode & _Folder)
{
  std::error_code ErrorCode;
  const auto      Options = std::filesystem::directory_options::skip_permission_denied;

  for (std::filesystem::directory_iterator Entry(_Folder.Record.AbsolutePath, Options, ErrorCode), End; Entry != End; Entry.increment(ErrorCode))
  {
    if (ErrorCode)
    {
      m_LastError = ErrorCode.message();
      break;
    }

    std::error_code EntryError;

    TAssetNode Node;
    Node.Name                = Entry->path().filename().string();
    Node.IsDirectory         = Entry->is_directory(EntryError);
    Node.Record.AbsolutePath = Entry->path().lexically_normal();
    Node.Record.RelativePath = _Folder.Record.RelativePath / Node.Name;

    if (Node.IsDirectory)
      ScanFolder(Node);
    else
    if (Entry->is_regular_file(EntryError))
      Node.Record.SizeBytes = Entry->file_size(EntryError);
    else
      continue;

    _Folder.Children.push_back(std::move(Node));
  }

  std::sort(_Folder.Children.begin(), _Folder.Children.end(), IsNodeOrderedBefore);
}

void CAssetManager::RemountResources()
{
  auto & ResourceGroups = Ogre::ResourceGroupManager::getSingleton();

  ResourceGroups.removeResourceLocation(m_AssetsRoot.string(), ResourceGroup);
  ResourceGroups.addResourceLocation(m_AssetsRoot.string(), "FileSystem", ResourceGroup, true, true);
}

std::filesystem::path CAssetManager::ToAbsolute(const std::filesystem::path & _RelativePath) const
{
  return (m_AssetsRoot / _RelativePath).lexically_normal();
}

} // namespace editor
