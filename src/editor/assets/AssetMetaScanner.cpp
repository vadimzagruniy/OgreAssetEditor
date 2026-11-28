#include "AssetMetaScanner.h"
#include "AssetMeta.h"
#include "AssetMetaSerializer.h"
#include "AssetNameRules.h"
#include "AssetPath.h"
#include "core/Log.h"
#include "core/PathUtf8.h"

#include <algorithm>
#include <system_error>
#include <utility>

namespace editor
{

namespace
{

std::string Quote(const std::filesystem::path & _RelativePath)
{
  return "'" + core::PathToUtf8(std::filesystem::path(_RelativePath.generic_wstring())) + "'";
}

bool IsPathPreferred(const std::filesystem::path & _Left, const std::filesystem::path & _Right)
{
  const auto LeftText  = _Left.generic_wstring();
  const auto RightText = _Right.generic_wstring();

  if (LeftText.size() != RightText.size())
    return LeftText.size() < RightText.size();

  return LeftText < RightText;
}

} // namespace

//
// Constructor/Destructor
//

CAssetMetaScanner::CAssetMetaScanner(std::shared_ptr<core::CLog> _Log):
  m_Log(std::move(_Log))
{
  // Empty
}

CAssetMetaScanner::~CAssetMetaScanner() = default;

//
// Interface
//

void CAssetMetaScanner::Scan(const std::filesystem::path & _AssetsRoot, TAssetIds & _Ids)
{
  m_AssetsRoot = _AssetsRoot;
  _Ids.clear();

  TPathSet Assets;
  TPathSet Metas;

  CollectFiles(Assets, Metas);
  AdoptOrphanMetas(Assets, Metas);
  ReadMetas(Assets, Metas, _Ids);
  ResolveDuplicateIds(_Ids);
  CreateMissingMetas(Assets, Metas, _Ids);

  m_KnownPaths.clear();
  for (const auto & [RelativePath, Id] : _Ids)
    m_KnownPaths[Id] = RelativePath;
}

bool CAssetMetaScanner::RegenerateIds(const std::filesystem::path & _AbsolutePath, std::string & _Error)
{
  std::error_code ErrorCode;

  if (!std::filesystem::is_directory(_AbsolutePath, ErrorCode))
  {
    const auto MetaPath = MakeMetaPath(_AbsolutePath);
    TAssetId   NewId;

    return !std::filesystem::exists(MetaPath, ErrorCode) || RegenerateId(MetaPath, NewId, _Error);
  }

  const auto Options = std::filesystem::directory_options::skip_permission_denied;

  for (std::filesystem::recursive_directory_iterator Entry(_AbsolutePath, Options, ErrorCode), End; Entry != End; Entry.increment(ErrorCode))
  {
    if (ErrorCode)
      break;

    std::error_code EntryError;
    if (!Entry->is_regular_file(EntryError) || !IsMetaPath(Entry->path()))
      continue;

    TAssetId NewId;
    if (!RegenerateId(Entry->path(), NewId, _Error))
      return false;
  }

  if (ErrorCode)
  {
    _Error = "Cannot read " + core::PathToUtf8(_AbsolutePath.filename()) + ": " + ErrorCode.message();
    return false;
  }

  return true;
}

//
// Implementation
//

void CAssetMetaScanner::CollectFiles(TPathSet & _Assets, TPathSet & _Metas) const
{
  std::error_code ErrorCode;
  const auto      Options = std::filesystem::directory_options::skip_permission_denied;

  for (std::filesystem::recursive_directory_iterator Entry(m_AssetsRoot, Options, ErrorCode), End; Entry != End; Entry.increment(ErrorCode))
  {
    if (ErrorCode)
      break;

    std::string       Reason;
    std::error_code   EntryError;
    const std::string Name = core::PathToUtf8(Entry->path().filename());

    if (Entry->is_directory(EntryError))
    {
      if (!IsValidAssetName(Name, Reason))
        Entry.disable_recursion_pending();

      continue;
    }

    if (!Entry->is_regular_file(EntryError))
      continue;

    const auto RelativePath = Entry->path().lexically_relative(m_AssetsRoot);

    if (IsMetaPath(RelativePath))
      _Metas.insert(RelativePath);
    else
    if (IsValidAssetName(Name, Reason))
      _Assets.insert(RelativePath);
  }
}

void CAssetMetaScanner::AdoptOrphanMetas(const TPathSet & _Assets, TPathSet & _Metas)
{
  TPathsByName OrphansByName;
  TPathsByName MetalessByName;

  for (const auto & MetaPath : _Metas)
  {
    const auto AssetPath = GetMetaAssetPath(MetaPath);
    if (_Assets.count(AssetPath) == 0)
      OrphansByName[core::PathToUtf8(AssetPath.filename())].push_back(MetaPath);
  }

  for (const auto & AssetPath : _Assets)
  {
    if (_Metas.count(MakeMetaPath(AssetPath)) == 0)
      MetalessByName[core::PathToUtf8(AssetPath.filename())].push_back(AssetPath);
  }

  for (const auto & [Name, Orphans] : OrphansByName)
  {
    const auto Metaless = MetalessByName.find(Name);
    if (Metaless == MetalessByName.end() || Orphans.size() != 1 || Metaless->second.size() != 1)
      continue;

    const auto & OldMetaPath = Orphans.front();
    const auto   NewMetaPath = MakeMetaPath(Metaless->second.front());

    std::error_code ErrorCode;
    std::filesystem::rename(ToAbsolute(OldMetaPath), ToAbsolute(NewMetaPath), ErrorCode);

    if (ErrorCode)
    {
      WarnOnce("Cannot move metadata " + Quote(OldMetaPath) + " to " + Quote(NewMetaPath) + ": " + ErrorCode.message());
      continue;
    }

    _Metas.erase(OldMetaPath);
    _Metas.insert(NewMetaPath);

    m_Log->Write(core::ELogLevel::Info, "Asset " + Quote(Metaless->second.front()) + " keeps its ID: metadata moved from " + Quote(OldMetaPath));
  }
}

void CAssetMetaScanner::ReadMetas(const TPathSet & _Assets, const TPathSet & _Metas, TAssetIds & _Ids)
{
  for (const auto & MetaPath : _Metas)
  {
    const auto AssetPath = GetMetaAssetPath(MetaPath);
    if (_Assets.count(AssetPath) == 0)
    {
      WarnOnce("Metadata " + Quote(MetaPath) + " has no asset next to it.");
      continue;
    }

    TAssetMeta  Meta;
    std::string Error;

    if (!LoadAssetMeta(ToAbsolute(MetaPath), Meta, Error))
    {
      WarnOnce("Asset " + Quote(AssetPath) + " has no ID: " + Error);
      continue;
    }

    _Ids[AssetPath] = Meta.Id;
  }
}

void CAssetMetaScanner::ResolveDuplicateIds(TAssetIds & _Ids)
{
  std::map<TAssetId, std::vector<std::filesystem::path>> PathsById;

  for (const auto & [RelativePath, Id] : _Ids)
    PathsById[Id].push_back(RelativePath);

  for (const auto & [Id, Paths] : PathsById)
  {
    if (Paths.size() < 2)
      continue;

    const auto & Keeper = ChooseIdKeeper(Id, Paths);

    for (const auto & RelativePath : Paths)
    {
      if (RelativePath == Keeper)
        continue;

      TAssetId    NewId;
      std::string Error;

      if (!RegenerateId(ToAbsolute(MakeMetaPath(RelativePath)), NewId, Error))
      {
        _Ids.erase(RelativePath);
        WarnOnce("Asset " + Quote(RelativePath) + " has the same ID as " + Quote(Keeper) + " and cannot get a new one: " + Error);
        continue;
      }

      _Ids[RelativePath] = NewId;

      m_Log->Write(core::ELogLevel::Warning, "Asset " + Quote(RelativePath) + " had the same ID as " + Quote(Keeper) + " and got a new ID.");
    }
  }
}

void CAssetMetaScanner::CreateMissingMetas(const TPathSet & _Assets, const TPathSet & _Metas, TAssetIds & _Ids)
{
  std::size_t CreatedCount = 0;

  for (const auto & AssetPath : _Assets)
  {
    const auto MetaPath = MakeMetaPath(AssetPath);
    if (_Metas.count(MetaPath) != 0)
      continue;

    TAssetMeta Meta;
    Meta.Id = TAssetId::Generate();

    std::string Error;
    if (!SaveAssetMeta(ToAbsolute(MetaPath), Meta, Error))
    {
      WarnOnce("Asset " + Quote(AssetPath) + " has no ID: " + Error);
      continue;
    }

    _Ids[AssetPath] = Meta.Id;
    ++CreatedCount;
  }

  if (CreatedCount == 1)
    m_Log->Write(core::ELogLevel::Info, "Created metadata for 1 asset.");
  else
  if (CreatedCount > 1)
    m_Log->Write(core::ELogLevel::Info, "Created metadata for " + std::to_string(CreatedCount) + " assets.");
}

const std::filesystem::path & CAssetMetaScanner::ChooseIdKeeper(const TAssetId & _Id, const std::vector<std::filesystem::path> & _Paths) const
{
  const auto KnownPath = m_KnownPaths.find(_Id);
  if (KnownPath != m_KnownPaths.end())
  {
    const auto Known = std::find(_Paths.begin(), _Paths.end(), KnownPath->second);
    if (Known != _Paths.end())
      return *Known;
  }

  return *std::min_element(_Paths.begin(), _Paths.end(), IsPathPreferred);
}

bool CAssetMetaScanner::RegenerateId(const std::filesystem::path & _MetaPath, TAssetId & _NewId, std::string & _Error) const
{
  TAssetMeta Meta;
  if (!LoadAssetMeta(_MetaPath, Meta, _Error))
    return false;

  Meta.Id = TAssetId::Generate();

  if (!SaveAssetMeta(_MetaPath, Meta, _Error))
    return false;

  _NewId = Meta.Id;

  return true;
}

void CAssetMetaScanner::WarnOnce(const std::string & _Message)
{
  if (m_ReportedWarnings.insert(_Message).second)
    m_Log->Write(core::ELogLevel::Warning, _Message);
}

std::filesystem::path CAssetMetaScanner::ToAbsolute(const std::filesystem::path & _RelativePath) const
{
  return (m_AssetsRoot / _RelativePath).lexically_normal();
}

} // namespace editor
