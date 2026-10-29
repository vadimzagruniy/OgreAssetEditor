#pragma once

#include <memory>
#include <string>

namespace editor
{

struct TAssetRecord;
struct TAssetTypeInfo;
class CEditorWorkspace;

class IAssetType
{
public: // Destructor

  virtual ~IAssetType();

public: // Interface

  virtual const TAssetTypeInfo & GetInfo() const noexcept = 0;

  virtual std::unique_ptr<CEditorWorkspace> OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const = 0;
};

} // namespace editor
