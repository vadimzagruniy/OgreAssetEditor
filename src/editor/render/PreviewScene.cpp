#include "PreviewScene.h"
#include "PreviewShape.h"
#include "RenderScene.h"

#include <OgreEntity.h>
#include <OgreMaterial.h>
#include <OgreMesh.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

namespace editor
{

namespace
{

const char * GetShapeMeshName(EPreviewShape _Shape)
{
  switch (_Shape)
  {
    case EPreviewShape::Cube:   return Ogre::SceneManager::PT_CUBE;
    case EPreviewShape::Plane:  return Ogre::SceneManager::PT_PLANE;
    case EPreviewShape::Sphere: break;
  }

  return Ogre::SceneManager::PT_SPHERE;
}

} // namespace

//
// Constructor/Destructor
//

CPreviewScene::CPreviewScene():
  m_RenderScene(std::make_unique<CRenderScene>()),
  m_Entity(nullptr)
{
  // Empty
}

CPreviewScene::~CPreviewScene() = default;

//
// Interface
//

CRenderScene & CPreviewScene::GetRenderScene() const noexcept
{
  return *m_RenderScene;
}

void CPreviewScene::SetMesh(const Ogre::MeshPtr & _Mesh)
{
  ShowEntity(m_RenderScene->GetSceneManager()->createEntity(_Mesh));
}

void CPreviewScene::SetShape(EPreviewShape _Shape)
{
  ShowEntity(m_RenderScene->GetSceneManager()->createEntity(GetShapeMeshName(_Shape)));
}

void CPreviewScene::SetMaterial(const Ogre::MaterialPtr & _Material)
{
  m_Material = _Material;

  if (m_Entity && m_Material)
    m_Entity->setMaterial(m_Material);
}

//
// Implementation
//

void CPreviewScene::ShowEntity(Ogre::Entity * _Entity)
{
  auto * ContentRoot = m_RenderScene->GetContentRoot();

  if (m_Entity)
  {
    ContentRoot->detachAllObjects();
    m_RenderScene->GetSceneManager()->destroyEntity(m_Entity);
  }

  m_Entity = _Entity;
  ContentRoot->attachObject(m_Entity);

  if (m_Material)
    m_Entity->setMaterial(m_Material);

  m_RenderScene->MarkContentReplaced();
}

} // namespace editor
