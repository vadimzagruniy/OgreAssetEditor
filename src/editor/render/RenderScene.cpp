#include "RenderScene.h"

#include <OgreCamera.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreLight.h>
#include <OgreMovableObject.h>
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

void MergeBounds(const Ogre::SceneNode & _Node, Ogre::AxisAlignedBox & _Bounds)
{
  for (const auto * Object : _Node.getAttachedObjects())
    _Bounds.merge(Object->getWorldBoundingBox(true));

  for (const auto * Child : _Node.getChildren())
    MergeBounds(static_cast<const Ogre::SceneNode &>(*Child), _Bounds);
}

} // namespace

//
// Constructor/Destructor
//

CRenderScene::CRenderScene():
  m_SceneManager(Ogre::Root::getSingleton().createSceneManager()),
  m_Camera(nullptr),
  m_CameraNode(nullptr),
  m_ContentRoot(nullptr),
  m_Width(0),
  m_Height(0),
  m_IsActive(true),
  m_ContentRevision(0)
{
  Ogre::RTShader::ShaderGenerator::getSingleton().addSceneManager(m_SceneManager);

  m_SceneManager->setAmbientLight(AmbientColour);

  m_ContentRoot = m_SceneManager->getRootSceneNode()->createChildSceneNode();

  m_Camera = m_SceneManager->createCamera("RenderSceneCamera");
  m_Camera->setAutoAspectRatio(true);

  m_CameraNode = m_SceneManager->getRootSceneNode()->createChildSceneNode();
  m_CameraNode->attachObject(m_Camera);
  m_CameraNode->attachObject(m_SceneManager->createLight(Ogre::Light::LT_DIRECTIONAL));
}

CRenderScene::~CRenderScene()
{
  DestroyRenderTarget();

  if (auto * ShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr())
    ShaderGenerator->removeSceneManager(m_SceneManager);

  Ogre::Root::getSingleton().destroySceneManager(m_SceneManager);
}

//
// Interface
//

void CRenderScene::Resize(unsigned int _Width, unsigned int _Height)
{
  if (_Width == 0 || _Height == 0)
    return;

  if (_Width == m_Width && _Height == m_Height)
    return;

  DestroyRenderTarget();
  CreateRenderTarget(_Width, _Height);
}

void CRenderScene::SetActive(bool _IsActive)
{
  m_IsActive = _IsActive;

  if (m_RenderTexture)
    m_RenderTexture->getBuffer()->getRenderTarget()->setActive(m_IsActive);
}

const Ogre::TexturePtr & CRenderScene::GetTexture() const noexcept
{
  return m_RenderTexture;
}

Ogre::SceneManager * CRenderScene::GetSceneManager() const noexcept
{
  return m_SceneManager;
}

Ogre::Camera * CRenderScene::GetCamera() const noexcept
{
  return m_Camera;
}

Ogre::SceneNode * CRenderScene::GetCameraNode() const noexcept
{
  return m_CameraNode;
}

Ogre::SceneNode * CRenderScene::GetContentRoot() const noexcept
{
  return m_ContentRoot;
}

Ogre::Ray CRenderScene::GetViewportRay(float _NormalizedX, float _NormalizedY) const
{
  return m_Camera->getCameraToViewportRay(_NormalizedX, _NormalizedY);
}

Ogre::AxisAlignedBox CRenderScene::GetContentBounds() const
{
  Ogre::AxisAlignedBox Bounds;
  MergeBounds(*m_ContentRoot, Bounds);

  return Bounds;
}

std::uint64_t CRenderScene::GetContentRevision() const noexcept
{
  return m_ContentRevision;
}

void CRenderScene::MarkContentReplaced()
{
  ++m_ContentRevision;
}

//
// Implementation
//

void CRenderScene::CreateRenderTarget(unsigned int _Width, unsigned int _Height)
{
  const Ogre::String TextureName = "RenderScene/" + m_SceneManager->getName();

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

void CRenderScene::DestroyRenderTarget()
{
  if (!m_RenderTexture)
    return;

  Ogre::TextureManager::getSingleton().remove(m_RenderTexture);
  m_RenderTexture.reset();

  m_Width  = 0;
  m_Height = 0;
}

} // namespace editor
