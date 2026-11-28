#pragma once

#include "AssetId.h"

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace editor::core
{
class CLog;
} // namespace editor::core

namespace editor
{

class CAssetMetaScanner
{
public: // Types

  using TAssetIds = std::map<std::filesystem::path, TAssetId>;

public: // Constructor/Destructor

  explicit CAssetMetaScanner(std::shared_ptr<core::CLog> _Log);

  ~CAssetMetaScanner();

public: // Interface

  void Scan(const std::filesystem::path & _AssetsRoot, TAssetIds & _Ids);

  bool RegenerateIds(const std::filesystem::path & _AbsolutePath, std::string & _Error);

private: // Types

  using TPathSet     = std::set<std::filesystem::path>;
  using TPathsByName = std::map<std::string, std::vector<std::filesystem::path>>;

private: // Implementation

  void CollectFiles(TPathSet & _Assets, TPathSet & _Metas) const;

  void AdoptOrphanMetas(const TPathSet & _Assets, TPathSet & _Metas);

  void ReadMetas(const TPathSet & _Assets, const TPathSet & _Metas, TAssetIds & _Ids);

  void ResolveDuplicateIds(TAssetIds & _Ids);

  void CreateMissingMetas(const TPathSet & _Assets, const TPathSet & _Metas, TAssetIds & _Ids);

  const std::filesystem::path & ChooseIdKeeper(const TAssetId & _Id, const std::vector<std::filesystem::path> & _Paths) const;

  bool RegenerateId(const std::filesystem::path & _MetaPath, TAssetId & _NewId, std::string & _Error) const;

  void WarnOnce(const std::string & _Message);

  std::filesystem::path ToAbsolute(const std::filesystem::path & _RelativePath) const;

private: // Attributes

  std::shared_ptr<core::CLog>               m_Log;
  std::filesystem::path                     m_AssetsRoot;
  std::map<TAssetId, std::filesystem::path> m_KnownPaths;
  std::set<std::string>                     m_ReportedWarnings;
};

} // namespace editor
