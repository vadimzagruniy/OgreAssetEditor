#pragma once

#include "editor/assets/AssetType.h"

#include <memory>

namespace editor
{

class CMaterialEditorController;

class CMaterialAssetType final:
  public IAssetType
{
public: // Constructor

  explicit CMaterialAssetType(std::shared_ptr<CMaterialEditorController> _Controller);

public: // IAssetType

  const TAssetTypeInfo & GetInfo() const noexcept override;

  std::unique_ptr<CEditorWorkspace> OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const override;

private: // Attributes

  std::shared_ptr<CMaterialEditorController> m_Controller;
};

} // namespace editor
