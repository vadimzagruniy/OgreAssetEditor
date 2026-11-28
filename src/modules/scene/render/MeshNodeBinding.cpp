#include "MeshNodeBinding.h"
#include "SceneBindingContext.h"
#include "SceneMeshCache.h"
#include "editor/assets/AssetDatabase.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetResolveResult.h"

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
  m_Assets(_Context.Assets),
  m_Entity(nullptr),
  m_IsMissing(false)
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
  const auto & MeshRef = _Node.GetMeshRef();

  TAssetRecord Record;
  const bool   IsMissing = !MeshRef.IsEmpty() && m_Assets.Resolve(MeshRef, Record) == EAssetResolveResult::Missing;
  const auto   MeshPath  = MeshRef.IsEmpty() || IsMissing ? std::string() : Record.RelativePath.generic_string();

  if (MeshPath == m_MeshPath && IsMissing == m_IsMissing)
    return true;

  DestroyEntity();

  m_MeshPath  = MeshPath;
  m_IsMissing = IsMissing;

  return CreateEntity(_Node, _Error);
}

//
// Implementation
//

bool CMeshNodeBinding::CreateEntity(const CMeshNode & _Node, std::string & _Error)
{
  if (m_IsMissing)
  {
    AttachEntity(m_SceneManager->createEntity(Ogre::SceneManager::PT_CUBE));

    _Error = "Mesh '" + _Node.GetMeshRef().PathHint + "' is missing from the project, a placeholder is shown.";
    return false;
  }

  if (m_MeshPath.empty())
    return true;

  Ogre::MeshPtr Mesh;
  if (!m_Meshes.Acquire(m_MeshPath, Mesh, _Error))
    return false;

  AttachEntity(m_SceneManager->createEntity(Mesh));

  return true;
}

void CMeshNodeBinding::AttachEntity(Ogre::Entity * _Entity)
{
  m_Entity = _Entity;
  m_Node->attachObject(m_Entity);
}

void CMeshNodeBinding::DestroyEntity()
{
  if (!m_Entity)
    return;

  m_Node->detachObject(m_Entity);
  m_SceneManager->destroyEntity(m_Entity);
  m_Entity = nullptr;
}

} // namespace editor
