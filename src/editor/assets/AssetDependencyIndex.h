#pragma once

#include "AssetDependencyEntry.h"
#include "AssetId.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

namespace editor::core
{
class CLog;
} // namespace editor::core

namespace editor
{

class CAssetDatabase;
class CAssetTypeRegistry;
struct TAssetRecord;

class CAssetDependencyIndex final
{
public: // Constructor/Destructor

  CAssetDependencyIndex(std::shared_ptr<const CAssetDatabase> _Assets, std::shared_ptr<const CAssetTypeRegistry> _AssetTypes, std::shared_ptr<core::CLog> _Log);

  ~CAssetDependencyIndex();

  CAssetDependencyIndex(const CAssetDependencyIndex &) = delete;

  CAssetDependencyIndex & operator=(const CAssetDependencyIndex &) = delete;

public: // Interface

  void Update();

  std::vector<TAssetId> GetReferrers(const TAssetId & _AssetId) const;

  std::vector<TAssetReference> GetDependencyRefs(const TAssetId & _AssetId) const;

private: // Implementation

  void UpdateEntry(const TAssetRecord & _Record, std::map<TAssetId, TAssetDependencyEntry> & _Entries) const;

  void RebuildReferrers();

private: // Attributes

  std::shared_ptr<const CAssetDatabase>     m_Assets;
  std::shared_ptr<const CAssetTypeRegistry> m_AssetTypes;
  std::shared_ptr<core::CLog>               m_Log;
  std::map<TAssetId, TAssetDependencyEntry> m_Entries;
  std::map<TAssetId, std::set<TAssetId>>    m_Referrers;
};

} // namespace editor
