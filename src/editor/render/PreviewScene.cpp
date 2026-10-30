#include "PreviewScene.h"
#include "OrbitCamera.h"
#include "PreviewShape.h"

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreLight.h>
#include <OgreMaterial.h>
#include <OgreMesh.h>
#include <OgreRenderTexture.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreShaderGenerator.h>
#include <OgreTextureManager.h>
#include <OgreViewport.h>

namespace editor
{

namespace
{

const Ogre::ColourValue BackgroundColour(0.18f, 0.19f, 0.21f);
const Ogre::ColourValue AmbientColour(0.35f, 0.35f, 0.35f);

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
  m_SceneManager(Ogre::Root::getSingleton().createSceneManager()),
  m_Camera(nullptr),
  m_MeshNode(nullptr),
  m_Entity(nullptr),
  m_Width(0),
  m_Height(0),
  m_IsActive(true)
{
  Ogre::RTShader::ShaderGenerator::getSingleton().addSceneManager(m_SceneManager);

  m_SceneManager->setAmbientLight(AmbientColour);

  m_MeshNode = m_SceneManager->getRootSceneNode()->createChildSceneNode();

  m_Camera = m_SceneManager->createCamera("PreviewCamera");
  m_Camera->setAutoAspectRatio(true);

  auto * CameraNode = m_SceneManager->getRootSceneNode()->createChildSceneNode();
  CameraNode->attachObject(m_Camera);
  CameraNode->attachObject(m_SceneManager->createLight(Ogre::Light::LT_DIRECTIONAL));

  m_OrbitCamera = std::make_unique<COrbitCamera>(CameraNode);
}

CPreviewScene::~CPreviewScene()
{
  DestroyRenderTarget();

  if (auto * ShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr())
    ShaderGenerator->removeSceneManager(m_SceneManager);

  Ogre::Root::getSingleton().destroySceneManager(m_SceneManager);
}

//
// Interface
//

void CPreviewScene::SetMesh(const Ogre::MeshPtr & _Mesh)
{
  ShowEntity(m_SceneManager->createEntity(_Mesh));
}

void CPreviewScene::SetShape(EPreviewShape _Shape)
{
  ShowEntity(m_SceneManager->createEntity(GetShapeMeshName(_Shape)));
}

void CPreviewScene::SetMaterial(const Ogre::MaterialPtr & _Material)
{
  m_Material = _Material;

  if (m_Entity && m_Material)
    m_Entity->setMaterial(m_Material);
}

void CPreviewScene::Resize(unsigned int _Width, unsigned int _Height)
{
  if (_Width == 0 || _Height == 0)
    return;

  if (_Width == m_Width && _Height == m_Height)
    return;

  DestroyRenderTarget();
  CreateRenderTarget(_Width, _Height);
}

void CPreviewScene::SetActive(bool _IsActive)
{
  m_IsActive = _IsActive;

  if (m_RenderTexture)
    m_RenderTexture->getBuffer()->getRenderTarget()->setActive(m_IsActive);
}

const Ogre::TexturePtr & CPreviewScene::GetTexture() const noexcept
{
  return m_RenderTexture;
}

COrbitCamera & CPreviewScene::GetCamera() noexcept
{
  return *m_OrbitCamera;
}

//
// Implementation
//

void CPreviewScene::ShowEntity(Ogre::Entity * _Entity)
{
  if (m_Entity)
  {
    m_MeshNode->detachAllObjects();
    m_SceneManager->destroyEntity(m_Entity);
  }

  m_Entity = _Entity;
  m_MeshNode->attachObject(m_Entity);

  if (m_Material)
    m_Entity->setMaterial(m_Material);

  const auto & Mesh   = m_Entity->getMesh();
  const float  Radius = Mesh->getBoundingSphereRadius();

  m_Camera->setNearClipDistance(Radius * 0.01f);
  m_Camera->setFarClipDistance(Radius * 100.0f);

  m_OrbitCamera->Focus(Mesh->getBounds().getCenter(), Radius);
}

void CPreviewScene::CreateRenderTarget(unsigned int _Width, unsigned int _Height)
{
  const Ogre::String TextureName = "PreviewScene/" + m_SceneManager->getName();

  m_RenderTexture = Ogre::TextureManager::getSingleton().createManual(TextureName, Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D, _Width, _Height, 0, Ogre::PF_BYTE_RGBA, Ogre::TU_RENDERTARGET);

  auto * RenderTarget = m_RenderTexture->getBuffer()->getRenderTarget();
  RenderTarget->setActive(m_IsActive);

  auto * Viewport = RenderTarget->addViewport(m_Camera);
  Viewport->setBackgroundColour(BackgroundColour);
  Viewport->setOverlaysEnabled(false);
  Viewport->setSkiesEnabled(false);

  m_Width  = _Width;
  m_Height = _Height;
}

void CPreviewScene::DestroyRenderTarget()
{
  if (!m_RenderTexture)
    return;

  Ogre::TextureManager::getSingleton().remove(m_RenderTexture);
  m_RenderTexture.reset();

  m_Width  = 0;
  m_Height = 0;
}

} // namespace editor
