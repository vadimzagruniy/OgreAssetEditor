#include "MeshAssetType.h"
#include "MeshDocument.h"
#include "MeshWorkspace.h"
#include "core/PathUtf8.h"
#include "editor/assets/AssetOpenResult.h"
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

EAssetOpenResult CMeshAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::unique_ptr<CEditorWorkspace> & _Workspace, std::string & _Error) const
{
  std::error_code FileError;
  if (!std::filesystem::is_regular_file(_Asset.AbsolutePath, FileError) || FileError)
  {
    _Error = "Mesh file is missing or inaccessible: " + core::PathToUtf8(_Asset.AbsolutePath);
    return EAssetOpenResult::Failed;
  }

  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File || _Asset.SizeBytes == 0)
  {
    _Error = "Mesh file is empty or unreadable: " + core::PathToUtf8(_Asset.AbsolutePath);
    return EAssetOpenResult::Failed;
  }

  auto Mesh = Ogre::MeshManager::getSingleton().load(_Asset.RelativePath.generic_string(), _ResourceGroup);
  if (!Mesh)
  {
    _Error = "OGRE returned an empty mesh: " + _Asset.RelativePath.string();
    return EAssetOpenResult::Failed;
  }

  _Workspace = std::make_unique<CMeshWorkspace>(std::make_shared<CMeshDocument>(_Asset, std::move(Mesh)));

  return EAssetOpenResult::OpenedInWorkspace;
}

} // namespace editor
