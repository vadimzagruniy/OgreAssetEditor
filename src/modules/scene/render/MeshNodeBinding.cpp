#include "MeshNodeBinding.h"
#include "SceneBindingContext.h"
#include "SceneMeshCache.h"

#include <OgreEntity.h>
#include <OgreMesh.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

namespace editor
{

//
// Constructor/Destructor
//

CMeshNodeBinding::CMeshNodeBinding(const TSceneBindingContext & _Context):
  m_SceneManager(_Context.SceneManager),
  m_Node(_Context.Node),
  m_Meshes(_Context.Meshes),
  m_Entity(nullptr)
{
  // Empty
}

CMeshNodeBinding::~CMeshNodeBinding()
{
  DestroyEntity();
}

//
// CSceneNodeBinding
//

bool CMeshNodeBinding::OnApply(const CMeshNode & _Node, std::string & _Error)
{
  const auto & MeshName = _Node.GetMeshName();
  if (MeshName == m_MeshName)
    return true;

  DestroyEntity();
  m_MeshName = MeshName;

  if (MeshName.empty())
    return true;

  Ogre::MeshPtr Mesh;
  if (!m_Meshes.Acquire(MeshName, Mesh, _Error))
    return false;

  m_Entity = m_SceneManager->createEntity(Mesh);
  m_Node->attachObject(m_Entity);

  return true;
}

//
// Implementation
//

void CMeshNodeBinding::DestroyEntity()
{
  if (!m_Entity)
    return;

  m_Node->detachObject(m_Entity);
  m_SceneManager->destroyEntity(m_Entity);
  m_Entity = nullptr;
}

} // namespace editor
