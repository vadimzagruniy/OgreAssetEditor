#include "PrimitiveNodeBinding.h"
#include "SceneBindingContext.h"

#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

namespace editor
{

namespace
{

const char * GetPrefabName(EPrimitiveShape _Shape)
{
  switch (_Shape)
  {
    case EPrimitiveShape::Cube:   return Ogre::SceneManager::PT_CUBE;
    case EPrimitiveShape::Sphere: return Ogre::SceneManager::PT_SPHERE;
    case EPrimitiveShape::Plane:  return Ogre::SceneManager::PT_PLANE;
  }

  return Ogre::SceneManager::PT_CUBE;
}

} // namespace

//
// Constructor/Destructor
//

CPrimitiveNodeBinding::CPrimitiveNodeBinding(const TSceneBindingContext & _Context):
  m_SceneManager(_Context.SceneManager),
  m_Node(_Context.Node),
  m_Entity(nullptr)
{
  // Empty
}

CPrimitiveNodeBinding::~CPrimitiveNodeBinding()
{
  DestroyEntity();
}

//
// CSceneNodeBinding
//

bool CPrimitiveNodeBinding::OnApply(const CPrimitiveNode & _Node, std::string & /*_Error*/)
{
  if (m_Shape == _Node.GetShape())
    return true;

  DestroyEntity();
  m_Shape = _Node.GetShape();

  m_Entity = m_SceneManager->createEntity(GetPrefabName(*m_Shape));
  m_Node->attachObject(m_Entity);

  return true;
}

//
// Implementation
//

void CPrimitiveNodeBinding::DestroyEntity()
{
  if (!m_Entity)
    return;

  m_Node->detachObject(m_Entity);
  m_SceneManager->destroyEntity(m_Entity);
  m_Entity = nullptr;
}

} // namespace editor
