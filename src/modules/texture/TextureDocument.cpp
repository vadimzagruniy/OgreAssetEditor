#include "TextureDocument.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/ResourceRelease.h"
#include "editor/documents/AssetProperty.h"

#include <OgreTexture.h>

#include <string>
#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CTextureDocument::CTextureDocument(TAssetRecord _Record, Ogre::TexturePtr _Texture):
  CAssetDocument(std::move(_Record), "Texture"),
  m_Texture(std::move(_Texture))
{
  // Empty
}

CTextureDocument::~CTextureDocument()
{
  ReleaseOgreResource(std::move(m_Texture));
}

//
// CAssetDocument
//

std::vector<TAssetProperty> CTextureDocument::GetProperties() const
{
  return {{"Width",      std::to_string(m_Texture->getWidth())  + " px"},
          {"Height",     std::to_string(m_Texture->getHeight()) + " px"},
          {"Mip levels", std::to_string(m_Texture->getNumMipmaps())    }};
}

//
// Interface
//

const Ogre::TexturePtr & CTextureDocument::GetTexture() const noexcept
{
  return m_Texture;
}

} // namespace editor
