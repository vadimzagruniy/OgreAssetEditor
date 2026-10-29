#include "TextureInstance.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/ResourceRelease.h"
#include "editor/workspaces/AssetProperty.h"

#include <OgreTexture.h>

#include <string>
#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CTextureInstance::CTextureInstance(TAssetRecord _Record, Ogre::TexturePtr _Texture):
  CAssetInstance(std::move(_Record), "Texture"),
  m_Texture(std::move(_Texture))
{
  // Empty
}

CTextureInstance::~CTextureInstance()
{
  ReleaseOgreResource(std::move(m_Texture));
}

//
// Interface
//

const Ogre::TexturePtr & CTextureInstance::GetTexture() const noexcept
{
  return m_Texture;
}

//
// CAssetInstance
//

std::vector<TAssetProperty> CTextureInstance::GetProperties() const
{
  return {{"Width",      std::to_string(m_Texture->getWidth())  + " px"},
          {"Height",     std::to_string(m_Texture->getHeight()) + " px"},
          {"Mip levels", std::to_string(m_Texture->getNumMipmaps())    }};
}

} // namespace editor
