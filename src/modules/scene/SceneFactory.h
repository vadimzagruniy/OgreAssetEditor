#pragma once

#include "editor/assets/AssetFactory.h"

#include <memory>

namespace editor
{

class CSceneNodeTypes;

class CSceneFactory final:
  public IAssetFactory
{
public: // Constructor

  explicit CSceneFactory(std::shared_ptr<const CSceneNodeTypes> _NodeTypes);

public: // IAssetFactory

  const TAssetFactoryInfo & GetInfo() const noexcept override;

  bool CreateContent(std::string & _Content, std::string & _Error) const override;

private: // Attributes

  std::shared_ptr<const CSceneNodeTypes> m_NodeTypes;
};

} // namespace editor
