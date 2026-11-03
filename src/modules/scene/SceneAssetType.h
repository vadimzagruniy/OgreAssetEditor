#pragma once

#include "editor/assets/AssetType.h"

#include <memory>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

class CSceneEditorController;

class CSceneAssetType final:
  public IAssetType
{
public: // Constructor

  CSceneAssetType(std::shared_ptr<CSceneEditorController> _Controller, std::shared_ptr<core::CLog> _Log);

public: // IAssetType

  const TAssetTypeInfo & GetInfo() const noexcept override;

  std::unique_ptr<CEditorWorkspace> OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const override;

private: // Attributes

  std::shared_ptr<CSceneEditorController> m_Controller;
  std::shared_ptr<core::CLog>             m_Log;
};

} // namespace editor
