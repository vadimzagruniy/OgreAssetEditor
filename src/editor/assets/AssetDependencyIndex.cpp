#include "AssetDependencyIndex.h"
#include "AssetDatabase.h"
#include "AssetRecord.h"
#include "AssetResolveResult.h"
#include "AssetType.h"
#include "AssetTypeRegistry.h"
#include "core/Log.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <system_error>
#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CAssetDependencyIndex::CAssetDependencyIndex(std::shared_ptr<const CAssetDatabase> _Assets, std::shared_ptr<const CAssetTypeRegistry> _AssetTypes, std::shared_ptr<core::CLog> _Log):
  m_Assets(std::move(_Assets)),
  m_AssetTypes(std::move(_AssetTypes)),
  m_Log(std::move(_Log))
{
  // Empty
}

CAssetDependencyIndex::~CAssetDependencyIndex() = default;

//
// Interface
//

void CAssetDependencyIndex::Update()
{
  std::map<TAssetId, TAssetDependencyEntry> Entries;

  for (const auto & [AssetId, Record] : m_Assets->GetRecords())
    UpdateEntry(Record, Entries);

  m_Entries = std::move(Entries);

  RebuildReferrers();
}

std::vector<TAssetId> CAssetDependencyIndex::GetReferrers(const TAssetId & _AssetId) const
{
  const auto Referrers = m_Referrers.find(_AssetId);
  if (Referrers == m_Referrers.end())
    return {};

  return std::vector<TAssetId>(Referrers->second.begin(), Referrers->second.end());
}

std::vector<TAssetReference> CAssetDependencyIndex::GetDependencyRefs(const TAssetId & _AssetId) const
{
  const auto Entry = m_Entries.find(_AssetId);

  return Entry == m_Entries.end() ? std::vector<TAssetReference>() : Entry->second.DependencyRefs;
}

const std::vector<TMissingAssetReference> & CAssetDependencyIndex::GetMissingRefs() const noexcept
{
  return m_MissingRefs;
}

std::vector<TAssetId> CAssetDependencyIndex::GetMissingReferrers(const TAssetReference & _MissingRef) const
{
  const auto MissingRef = std::find_if(m_MissingRefs.begin(), m_MissingRefs.end(), [&_MissingRef](const TMissingAssetReference & _Entry)
  {
    return _Entry.AssetRef.IsSameAsset(_MissingRef);
  });

  return MissingRef == m_MissingRefs.end() ? std::vector<TAssetId>() : MissingRef->ReferrerIds;
}

//
// Implementation
//

void CAssetDependencyIndex::UpdateEntry(const TAssetRecord & _Record, std::map<TAssetId, TAssetDependencyEntry> & _Entries) const
{
  const auto AssetType = m_AssetTypes->FindByExtension(_Record.RelativePath.extension().string());
  if (!AssetType)
    return;

  std::error_code ErrorCode;

  TAssetDependencyEntry Entry;
  Entry.WriteTime = std::filesystem::last_write_time(_Record.AbsolutePath, ErrorCode);
  Entry.SizeBytes = std::filesystem::file_size(_Record.AbsolutePath, ErrorCode);

  const auto Cached = m_Entries.find(_Record.Id);
  if (Cached != m_Entries.end() && Cached->second.WriteTime == Entry.WriteTime && Cached->second.SizeBytes == Entry.SizeBytes)
  {
    _Entries.emplace(_Record.Id, Cached->second);
    return;
  }

  std::string Error;
  if (!AssetType->CollectDependencies(_Record, Entry.DependencyRefs, Error))
  {
    Entry.DependencyRefs.clear();
    m_Log->Write(core::ELogLevel::Warning, "Cannot read dependencies of '" + _Record.RelativePath.generic_string() + "': " + Error);
  }

  _Entries.emplace(_Record.Id, std::move(Entry));
}

void CAssetDependencyIndex::RebuildReferrers()
{
  m_Referrers.clear();
  m_MissingRefs.clear();

  for (const auto & [AssetId, Entry] : m_Entries)
  {
    for (const auto & DependencyRef : Entry.DependencyRefs)
    {
      TAssetRecord Dependency;
      if (m_Assets->Resolve(DependencyRef, Dependency) == EAssetResolveResult::Missing)
      {
        AddMissingRef(DependencyRef, AssetId);
        continue;
      }

      if (Dependency.Id != AssetId)
        m_Referrers[Dependency.Id].insert(AssetId);
    }
  }
}

void CAssetDependencyIndex::AddMissingRef(const TAssetReference & _MissingRef, const TAssetId & _ReferrerId)
{
  auto MissingRef = std::find_if(m_MissingRefs.begin(), m_MissingRefs.end(), [&_MissingRef](const TMissingAssetReference & _Entry)
  {
    return _Entry.AssetRef.IsSameAsset(_MissingRef);
  });

  if (MissingRef == m_MissingRefs.end())
  {
    m_MissingRefs.push_back({_MissingRef, {}});
    MissingRef = std::prev(m_MissingRefs.end());
  }

  if (std::find(MissingRef->ReferrerIds.begin(), MissingRef->ReferrerIds.end(), _ReferrerId) == MissingRef->ReferrerIds.end())
    MissingRef->ReferrerIds.push_back(_ReferrerId);
}

} // namespace editor
