#include "MeshAssetType.h"
#include "MeshInstance.h"
#include "MeshWorkspace.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetTypeInfo.h"

#include <OgreMesh.h>
#include <OgreMeshManager.h>

#include <fstream>
#include <system_error>

namespace editor
{

//
// IAssetType
//

const TAssetTypeInfo & CMeshAssetType::GetInfo() const noexcept
{
  static const TAssetTypeInfo Info{"mesh", "Mesh", {".mesh"}, true};
  return Info;
}

std::unique_ptr<CEditorWorkspace> CMeshAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const
{
  std::error_code FileError;
  if (!std::filesystem::is_regular_file(_Asset.AbsolutePath, FileError) || FileError)
  {
    _Error = "Mesh file is missing or inaccessible: " + _Asset.AbsolutePath.string();
    return {};
  }

  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File || _Asset.SizeBytes == 0)
  {
    _Error = "Mesh file is empty or unreadable: " + _Asset.AbsolutePath.string();
    return {};
  }

  auto Mesh = Ogre::MeshManager::getSingleton().load(_Asset.RelativePath.generic_string(), _ResourceGroup);
  if (!Mesh)
  {
    _Error = "OGRE returned an empty mesh: " + _Asset.RelativePath.string();
    return {};
  }

  return std::make_unique<CMeshWorkspace>(std::make_unique<CMeshInstance>(_Asset, std::move(Mesh)));
}

} // namespace editor
