#include "SceneAssetType.h"
#include "SceneInstance.h"
#include "SceneSerializer.h"
#include "SceneWorkspace.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetTypeInfo.h"

#include <fstream>
#include <iterator>
#include <utility>

namespace editor
{

//
// IAssetType
//

const TAssetTypeInfo & CSceneAssetType::GetInfo() const noexcept
{
  static const TAssetTypeInfo Info{"scene", "Scene", {".oscene"}, true};
  return Info;
}

std::unique_ptr<CEditorWorkspace> CSceneAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & /*_ResourceGroup*/, std::string & _Error) const
{
  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File)
  {
    _Error = "Could not open scene file: " + _Asset.AbsolutePath.string();
    return {};
  }

  const std::string Text((std::istreambuf_iterator<char>(File)), {});

  auto Instance = std::make_unique<CSceneInstance>(_Asset);
  if (!ReadScene(Text, *Instance, _Error))
  {
    _Error = _Asset.RelativePath.generic_string() + ": " + _Error;
    return {};
  }

  return std::make_unique<CSceneWorkspace>(std::move(Instance));
}

} // namespace editor
