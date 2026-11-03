#include "SceneFactory.h"
#include "SceneInstance.h"
#include "SceneSerializer.h"
#include "editor/assets/AssetFactoryInfo.h"
#include "editor/assets/AssetRecord.h"

namespace editor
{

//
// IAssetFactory
//

const TAssetFactoryInfo & CSceneFactory::GetInfo() const noexcept
{
  static const TAssetFactoryInfo Info{"scene.empty", "scene", "Scene", "New Scene"};
  return Info;
}

bool CSceneFactory::CreateContent(std::string & _Content, std::string & /*_Error*/) const
{
  const CSceneInstance EmptyScene(TAssetRecord{}, std::string());

  _Content = WriteScene(EmptyScene);

  return true;
}

} // namespace editor
