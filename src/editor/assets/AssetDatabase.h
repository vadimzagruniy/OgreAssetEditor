#pragma once

#include "AssetId.h"
#include "AssetRecord.h"

#include <cstdint>
#include <filesystem>
#include <map>

namespace editor
{

enum class EAssetResolveResult;
struct TAssetNode;
struct TAssetReference;

class CAssetDatabase final
{
public: // Constructor/Destructor

  CAssetDatabase();

  ~CAssetDatabase();

  CAssetDatabase(const CAssetDatabase &) = delete;

  CAssetDatabase & operator=(const CAssetDatabase &) = delete;

public: // Interface

  void Rebuild(const TAssetNode & _Root);

  std::uint64_t GetRevision() const noexcept;

  const std::map<TAssetId, TAssetRecord> & GetRecords() const noexcept;

  bool FindById(const TAssetId & _Id, TAssetRecord & _Record) const;

  bool FindByPath(const std::filesystem::path & _RelativePath, TAssetRecord & _Record) const;

  EAssetResolveResult Resolve(const TAssetReference & _AssetRef, TAssetRecord & _Record) const;

  bool MakeReference(const std::filesystem::path & _RelativePath, TAssetReference & _AssetRef) const;

  bool RefreshReference(TAssetReference & _AssetRef) const;

private: // Implementation

  void AddNode(const TAssetNode & _Node);

private: // Attributes

  std::map<TAssetId, TAssetRecord>          m_Records;
  std::map<std::filesystem::path, TAssetId> m_IdsByPath;
  std::uint64_t                             m_Revision;
};

} // namespace editor
