#pragma once

#include "editor/assets/AssetType.h"

namespace editor
{

class CTextAssetType final:
  public IAssetType
{
public: // IAssetType

  const TAssetTypeInfo & GetInfo() const noexcept override;

  std::unique_ptr<CEditorWorkspace> OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const override;
};

} // namespace editor
