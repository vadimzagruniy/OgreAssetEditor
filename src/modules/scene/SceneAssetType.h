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
class CSceneNodeBindings;
class CSceneNodeTypes;

class CSceneAssetType final:
  public IAssetType
{
public: // Constructor

  CSceneAssetType(std::shared_ptr<CSceneEditorController> _Controller, std::shared_ptr<const CSceneNodeTypes> _NodeTypes, std::shared_ptr<const CSceneNodeBindings> _NodeBindings, std::shared_ptr<core::CLog> _Log);

public: // IAssetType

  const TAssetTypeInfo & GetInfo() const noexcept override;

  std::unique_ptr<CEditorWorkspace> OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const override;

private: // Attributes

  std::shared_ptr<CSceneEditorController>   m_Controller;
  std::shared_ptr<const CSceneNodeTypes>    m_NodeTypes;
  std::shared_ptr<const CSceneNodeBindings> m_NodeBindings;
  std::shared_ptr<core::CLog>               m_Log;
};

} // namespace editor
