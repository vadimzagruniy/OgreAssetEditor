#pragma once

#include "editor/assets/AssetFactory.h"

namespace editor
{

class CSceneFactory final:
  public IAssetFactory
{
public: // IAssetFactory

  const TAssetFactoryInfo & GetInfo() const noexcept override;

  bool CreateContent(std::string & _Content, std::string & _Error) const override;
};

} // namespace editor
