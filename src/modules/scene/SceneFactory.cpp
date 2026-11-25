#include "SceneFactory.h"
#include "SceneSerializer.h"
#include "editor/assets/AssetFactoryInfo.h"
#include "model/SceneAnimation.h"
#include "model/SceneGraph.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CSceneFactory::CSceneFactory(std::shared_ptr<const CSceneNodeTypes> _NodeTypes):
  m_NodeTypes(std::move(_NodeTypes))
{
  // Empty
}

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
  const CSceneGraph EmptyScene(m_NodeTypes);

  _Content = WriteScene(EmptyScene, {});

  return true;
}

} // namespace editor
