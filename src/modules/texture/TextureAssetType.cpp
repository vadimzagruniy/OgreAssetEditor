#include "TextureAssetType.h"
#include "TextureDocument.h"
#include "TextureWorkspace.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetTypeInfo.h"
#include "core/PathUtf8.h"

#include <OgreTextureManager.h>

#include <fstream>
#include <system_error>

namespace editor
{

//
// IAssetType
//

const TAssetTypeInfo & CTextureAssetType::GetInfo() const noexcept
{
  static const TAssetTypeInfo Info{"texture", "Texture", {".bmp", ".dds", ".png", ".jpg", ".jpeg", ".tga"}, true};
  return Info;
}

std::unique_ptr<CEditorWorkspace> CTextureAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const
{
  std::error_code FileError;
  if (!std::filesystem::is_regular_file(_Asset.AbsolutePath, FileError) || FileError)
  {
    _Error = "Texture file is missing or inaccessible: " + core::PathToUtf8(_Asset.AbsolutePath);
    return {};
  }

  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File || _Asset.SizeBytes == 0)
  {
    _Error = "Texture file is empty or unreadable: " + core::PathToUtf8(_Asset.AbsolutePath);
    return {};
  }

  auto Texture = Ogre::TextureManager::getSingleton().load(_Asset.RelativePath.generic_string(), _ResourceGroup, Ogre::TEX_TYPE_2D);
  if (!Texture)
  {
    _Error = "OGRE returned an empty texture: " + _Asset.RelativePath.string();
    return {};
  }

  return std::make_unique<CTextureWorkspace>(std::make_shared<CTextureDocument>(_Asset, std::move(Texture)));
}

} // namespace editor
