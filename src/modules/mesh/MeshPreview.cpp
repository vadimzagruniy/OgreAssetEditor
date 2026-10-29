#include "MeshPreview.h"
#include "OrbitCamera.h"

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreLight.h>
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

} // namespace

//
// Constructor/Destructor
//

CMeshPreview::CMeshPreview(const Ogre::MeshPtr & _Mesh):
  m_SceneManager(Ogre::Root::getSingleton().createSceneManager()),
  m_Camera(nullptr),
  m_Width(0),
  m_Height(0),
  m_IsActive(true)
{
  Ogre::RTShader::ShaderGenerator::getSingleton().addSceneManager(m_SceneManager);

  m_SceneManager->setAmbientLight(AmbientColour);

  auto * MeshNode = m_SceneManager->getRootSceneNode()->createChildSceneNode();
  MeshNode->attachObject(m_SceneManager->createEntity(_Mesh));

  m_Camera = m_SceneManager->createCamera("PreviewCamera");
  m_Camera->setAutoAspectRatio(true);

  auto * CameraNode = m_SceneManager->getRootSceneNode()->createChildSceneNode();
  CameraNode->attachObject(m_Camera);
  CameraNode->attachObject(m_SceneManager->createLight(Ogre::Light::LT_DIRECTIONAL));

  const float Radius = _Mesh->getBoundingSphereRadius();

  m_Camera->setNearClipDistance(Radius * 0.01f);
  m_Camera->setFarClipDistance(Radius * 100.0f);

  m_OrbitCamera = std::make_unique<COrbitCamera>(CameraNode);
  m_OrbitCamera->Focus(_Mesh->getBounds().getCenter(), Radius);
}

CMeshPreview::~CMeshPreview()
{
  DestroyRenderTarget();

  if (auto * ShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr())
    ShaderGenerator->removeSceneManager(m_SceneManager);

  Ogre::Root::getSingleton().destroySceneManager(m_SceneManager);
}

//
// Interface
//

void CMeshPreview::Resize(unsigned int _Width, unsigned int _Height)
{
  if (_Width == 0 || _Height == 0)
    return;

  if (_Width == m_Width && _Height == m_Height)
    return;

  DestroyRenderTarget();
  CreateRenderTarget(_Width, _Height);
}

void CMeshPreview::SetActive(bool _IsActive)
{
  m_IsActive = _IsActive;

  if (m_RenderTexture)
    m_RenderTexture->getBuffer()->getRenderTarget()->setActive(m_IsActive);
}

const Ogre::TexturePtr & CMeshPreview::GetTexture() const noexcept
{
  return m_RenderTexture;
}

COrbitCamera & CMeshPreview::GetCamera() noexcept
{
  return *m_OrbitCamera;
}

//
// Implementation
//

void CMeshPreview::CreateRenderTarget(unsigned int _Width, unsigned int _Height)
{
  const Ogre::String TextureName = "MeshPreview/" + m_SceneManager->getName();

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

void CMeshPreview::DestroyRenderTarget()
{
  if (!m_RenderTexture)
    return;

  Ogre::TextureManager::getSingleton().remove(m_RenderTexture);
  m_RenderTexture.reset();

  m_Width  = 0;
  m_Height = 0;
}

} // namespace editor
