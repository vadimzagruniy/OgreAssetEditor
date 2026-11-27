#pragma once

#include "editor/assets/AssetType.h"

namespace editor
{

class CTextureAssetType final:
  public IAssetType
{
public: // IAssetType

  const TAssetTypeInfo & GetInfo() const noexcept override;

  EAssetOpenResult OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::unique_ptr<CEditorWorkspace> & _Workspace, std::string & _Error) const override;
};

} // namespace editor
