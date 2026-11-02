#pragma once

#include "editor/workspaces/AssetInstance.h"

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

namespace editor
{

class CTextureInstance final:
  public CAssetInstance
{
public: // Constructor/Destructor

  CTextureInstance(TAssetRecord _Record, Ogre::TexturePtr _Texture);

  ~CTextureInstance() override;

public: // CAssetInstance

  std::vector<TAssetProperty> GetProperties() const override;

public: // Interface

  const Ogre::TexturePtr & GetTexture() const noexcept;

private: // Attributes

  Ogre::TexturePtr m_Texture;
};

} // namespace editor
