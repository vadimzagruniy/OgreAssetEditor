#pragma once

#include <memory>
#include <string>
#include <vector>

namespace editor
{

enum class EAssetOpenResult;
struct TAssetMenuEntry;
struct TAssetRecord;
struct TAssetReference;
struct TAssetTypeInfo;
class CEditorWorkspace;

class IAssetType
{
public: // Destructor

  virtual ~IAssetType();

public: // Interface

  virtual const TAssetTypeInfo & GetInfo() const noexcept = 0;

  virtual EAssetOpenResult OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::unique_ptr<CEditorWorkspace> & _Workspace, std::string & _Error) const = 0;

  virtual void GetContextActions(const TAssetRecord & _Asset, std::vector<TAssetMenuEntry> & _Entries) const;

  virtual bool CollectDependencies(const TAssetRecord & _Asset, std::vector<TAssetReference> & _DependencyRefs, std::string & _Error) const;

  virtual bool ReplaceDependency(const TAssetRecord & _Asset, const TAssetReference & _OldRef, const TAssetReference & _NewRef, bool & _IsChanged, std::string & _Error) const;
};

} // namespace editor
