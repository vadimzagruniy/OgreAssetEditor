#include "AssetManager.h"
#include "AssetDatabase.h"
#include "AssetDependencyIndex.h"
#include "AssetFactory.h"
#include "AssetFactoryInfo.h"
#include "AssetFactoryRegistry.h"
#include "AssetMetaScanner.h"
#include "AssetNameRules.h"
#include "AssetPath.h"
#include "AssetReference.h"
#include "AssetType.h"
#include "AssetTypeInfo.h"
#include "AssetTypeRegistry.h"
#include "core/Log.h"
#include "core/PathUtf8.h"
#include "core/WriteFileAtomic.h"

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

CAssetManager::CAssetManager(std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<CAssetFactoryRegistry> _AssetFactories, std::shared_ptr<CAssetDatabase> _Database, std::shared_ptr<CAssetDependencyIndex> _Dependencies, std::shared_ptr<core::CLog> _Log):
  m_AssetTypes(std::move(_AssetTypes)),
  m_AssetFactories(std::move(_AssetFactories)),
  m_Database(std::move(_Database)),
  m_Dependencies(std::move(_Dependencies)),
  m_Log(std::move(_Log)),
  m_MetaScanner(std::make_unique<CAssetMetaScanner>(m_Log)),
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

  TAssetIds Ids;
  m_MetaScanner->Scan(m_AssetsRoot, Ids);

  ScanFolder(m_RootNode, Ids);
  m_Database->Rebuild(m_RootNode);
  m_Dependencies->Update();

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

  ResourceGroups.addResourceLocation(core::PathToUtf8(m_AssetsRoot), "FileSystem", ResourceGroup, true, true);

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
    _Error = "Cannot create folder: " + (ErrorCode ? ErrorCode.message() : core::PathToUtf8(AbsolutePath));
    return false;
  }

  _CreatedPath = _ParentFolder / AbsolutePath.filename();

  Refresh();

  return true;
}

bool CAssetManager::CreateAsset(const std::string & _FactoryIdentifier, const std::filesystem::path & _Folder, const std::string & _Name, std::filesystem::path & _CreatedPath, std::string & _Error)
{
  std::shared_ptr<IAssetFactory> Factory;
  std::string                    Extension;

  if (!CheckAssetName(_FactoryIdentifier, _Folder, _Name, _Error) || !FindFactory(_FactoryIdentifier, Factory, Extension, _Error))
    return false;

  std::string Content;
  if (!Factory->CreateContent(Content, _Error))
    return false;

  const std::filesystem::path RelativePath = _Folder / (_Name + Extension);

  if (!core::WriteFileAtomic(ToAbsolute(RelativePath), Content, _Error))
    return false;

  _CreatedPath = RelativePath;

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

  if (!IsValidAssetName(_NewName, _Error))
    return false;

  const std::filesystem::path NewPath = _Path.parent_path() / _NewName;
  if (NewPath == _Path)
  {
    _RenamedPath = _Path;
    return true;
  }

  std::error_code ErrorCode;
  const bool      IsSameFile = std::filesystem::equivalent(ToAbsolute(_Path), ToAbsolute(NewPath), ErrorCode);

  if (!IsSameFile && std::filesystem::exists(ToAbsolute(NewPath), ErrorCode))
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

  if (!RenameMeta(_Path, NewPath, _Error))
  {
    std::error_code RollbackError;
    std::filesystem::rename(ToAbsolute(NewPath), ToAbsolute(_Path), RollbackError);
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

  std::string MetaError;
  if (!DuplicateMeta(SourcePath, TargetPath, MetaError))
    m_Log->Write(core::ELogLevel::Warning, "Duplicate of " + _Path.generic_string() + " will get a new ID on refresh: " + MetaError);

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

  const bool IsDirectory = GetNode(_Path).IsDirectory;

  std::error_code ErrorCode;
  std::filesystem::remove_all(ToAbsolute(_Path), ErrorCode);

  if (!ErrorCode && !IsDirectory)
    std::filesystem::remove(MakeMetaPath(ToAbsolute(_Path)), ErrorCode);

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

bool CAssetManager::ReplaceReferences(const TAssetReference & _MissingRef, const std::filesystem::path & _ReplacementPath, std::vector<std::filesystem::path> & _ChangedPaths, std::string & _Error)
{
  TAssetReference NewRef;
  if (!m_Database->MakeReference(_ReplacementPath, NewRef))
  {
    _Error = "'" + _ReplacementPath.generic_string() + "' has no asset ID in the project.";
    return false;
  }

  m_Dependencies->Update();

  std::string Errors;

  for (const auto & ReferrerId : m_Dependencies->GetMissingReferrers(_MissingRef))
  {
    TAssetRecord Referrer;
    if (!m_Database->FindById(ReferrerId, Referrer))
      continue;

    const auto AssetType = m_AssetTypes->FindByExtension(Referrer.RelativePath.extension().string());
    if (!AssetType)
      continue;

    bool        IsChanged = false;
    std::string Error;

    if (!AssetType->ReplaceDependency(Referrer, _MissingRef, NewRef, IsChanged, Error))
      Errors += (Errors.empty() ? "" : "; ") + Referrer.RelativePath.generic_string() + ": " + Error;
    else
    if (IsChanged)
      _ChangedPaths.push_back(Referrer.RelativePath);
  }

  Refresh();

  if (!Errors.empty())
  {
    _Error = Errors;
    return false;
  }

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

std::string CAssetManager::MakeAssetName(const std::string & _FactoryIdentifier, const std::filesystem::path & _Folder) const
{
  std::shared_ptr<IAssetFactory> Factory;
  std::string                    Extension;
  std::string                    Error;

  if (!FindFactory(_FactoryIdentifier, Factory, Extension, Error))
    return {};

  return MakeUniquePath(ToAbsolute(_Folder), Factory->GetInfo().DefaultName, Extension).stem().string();
}

bool CAssetManager::CheckAssetName(const std::string & _FactoryIdentifier, const std::filesystem::path & _Folder, const std::string & _Name, std::string & _Error) const
{
  if (!HasNode(_Folder) || !GetNode(_Folder).IsDirectory)
  {
    _Error = "Folder does not exist: " + _Folder.generic_string();
    return false;
  }

  if (!IsValidAssetName(_Name, _Error))
    return false;

  std::shared_ptr<IAssetFactory> Factory;
  std::string                    Extension;

  if (!FindFactory(_FactoryIdentifier, Factory, Extension, _Error))
    return false;

  std::error_code ErrorCode;
  if (std::filesystem::exists(ToAbsolute(_Folder / (_Name + Extension)), ErrorCode))
  {
    _Error = "'" + _Name + Extension + "' already exists in this folder.";
    return false;
  }

  return true;
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

void CAssetManager::ScanFolder(TAssetNode & _Folder, const TAssetIds & _Ids)
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

    std::string       Reason;
    std::error_code   EntryError;
    const std::string Name = core::PathToUtf8(Entry->path().filename());

    if (IsMetaPath(Entry->path()) && Entry->is_regular_file(EntryError))
      continue;

    if (!IsValidAssetName(Name, Reason))
    {
      ReportInvalidEntry(Entry->path(), Reason);
      continue;
    }

    TAssetNode Node;
    Node.Name                = Name;
    Node.IsDirectory         = Entry->is_directory(EntryError);
    Node.Record.AbsolutePath = Entry->path().lexically_normal();
    Node.Record.RelativePath = _Folder.Record.RelativePath / Node.Name;

    if (Node.IsDirectory)
      ScanFolder(Node, _Ids);
    else
    if (Entry->is_regular_file(EntryError))
      Node.Record.SizeBytes = Entry->file_size(EntryError);
    else
      continue;

    if (const auto Id = _Ids.find(Node.Record.RelativePath); Id != _Ids.end())
      Node.Record.Id = Id->second;

    _Folder.Children.push_back(std::move(Node));
  }

  std::sort(_Folder.Children.begin(), _Folder.Children.end(), IsNodeOrderedBefore);
}

bool CAssetManager::RenameMeta(const std::filesystem::path & _Path, const std::filesystem::path & _NewPath, std::string & _Error)
{
  std::error_code ErrorCode;

  const auto OldMetaPath = MakeMetaPath(ToAbsolute(_Path));
  if (std::filesystem::is_directory(ToAbsolute(_NewPath), ErrorCode) || !std::filesystem::exists(OldMetaPath, ErrorCode))
    return true;

  std::filesystem::rename(OldMetaPath, MakeMetaPath(ToAbsolute(_NewPath)), ErrorCode);
  if (ErrorCode)
  {
    _Error = "Cannot rename metadata of " + _Path.generic_string() + ": " + ErrorCode.message();
    return false;
  }

  return true;
}

bool CAssetManager::DuplicateMeta(const std::filesystem::path & _SourcePath, const std::filesystem::path & _TargetPath, std::string & _Error)
{
  std::error_code ErrorCode;

  const auto SourceMetaPath = MakeMetaPath(_SourcePath);
  if (!std::filesystem::is_directory(_SourcePath, ErrorCode) && std::filesystem::exists(SourceMetaPath, ErrorCode))
  {
    std::filesystem::copy_file(SourceMetaPath, MakeMetaPath(_TargetPath), std::filesystem::copy_options::overwrite_existing, ErrorCode);
    if (ErrorCode)
    {
      _Error = ErrorCode.message();
      return false;
    }
  }

  return m_MetaScanner->RegenerateIds(_TargetPath, _Error);
}

void CAssetManager::ReportInvalidEntry(const std::filesystem::path & _Path, const std::string & _Reason)
{
  if (!m_ReportedInvalidPaths.insert(_Path).second)
    return;

  const std::string RelativePath = core::PathToUtf8(_Path.lexically_relative(m_AssetsRoot));

  m_Log->Write(core::ELogLevel::Warning, "Skipped '" + RelativePath + "': " + _Reason);
}

void CAssetManager::RemountResources()
{
  auto & ResourceGroups = Ogre::ResourceGroupManager::getSingleton();

  const std::string Location = core::PathToUtf8(m_AssetsRoot);

  ResourceGroups.removeResourceLocation(Location, ResourceGroup);
  ResourceGroups.addResourceLocation(Location, "FileSystem", ResourceGroup, true, true);
}

std::filesystem::path CAssetManager::ToAbsolute(const std::filesystem::path & _RelativePath) const
{
  return (m_AssetsRoot / _RelativePath).lexically_normal();
}

bool CAssetManager::FindFactory(const std::string & _FactoryIdentifier, std::shared_ptr<IAssetFactory> & _Factory, std::string & _Extension, std::string & _Error) const
{
  _Factory = m_AssetFactories->FindByIdentifier(_FactoryIdentifier);
  if (!_Factory)
  {
    _Error = "Unknown asset factory: " + _FactoryIdentifier;
    return false;
  }

  const auto & TypeIdentifier = _Factory->GetInfo().TypeIdentifier;
  const auto & AssetTypes     = m_AssetTypes->GetTypes();

  const auto AssetTypeIterator = AssetTypes.find(TypeIdentifier);
  if (AssetTypeIterator == AssetTypes.end() || AssetTypeIterator->second->GetInfo().Extensions.empty())
  {
    _Error = "Asset factory '" + _FactoryIdentifier + "' creates an unknown asset type: " + TypeIdentifier;
    return false;
  }

  _Extension = AssetTypeIterator->second->GetInfo().Extensions.front();

  return true;
}

} // namespace editor
