#pragma once

#include "editor/documents/AssetDocument.h"

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

namespace editor
{

class CTextureDocument final:
  public CAssetDocument
{
public: // Constructor/Destructor

  CTextureDocument(TAssetRecord _Record, Ogre::TexturePtr _Texture);

  ~CTextureDocument() override;

public: // CAssetDocument

  std::vector<TAssetProperty> GetProperties() const override;

public: // Interface

  const Ogre::TexturePtr & GetTexture() const noexcept;

private: // Attributes

  Ogre::TexturePtr m_Texture;
};

} // namespace editor
