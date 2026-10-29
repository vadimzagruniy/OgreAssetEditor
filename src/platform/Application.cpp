#include "Application.h"
#include "core/Log.h"
#include "editor/Editor.h"
#include "editor/assets/AssetManager.h"

#include <OgreCamera.h>
#include <OgreImGuiOverlay.h>
#include <OgreOverlayManager.h>
#include <OgreOverlaySystem.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreShaderGenerator.h>
#include <OgreViewport.h>
#include <imgui.h>

#include <utility>

namespace editor::platform
{

//
// Constructor/Destructor
//

CApplication::CApplication(std::filesystem::path _ProjectRoot):
  OgreBites::ApplicationContext("Ogre Resource Editor"),
  m_Editor(std::make_unique<CEditor>(std::move(_ProjectRoot)))
{
  m_Editor->SetQuitCallback([this] { getRoot()->queueEndRendering(); });
}

CApplication::~CApplication() = default;

//
// ApplicationContext
//

bool CApplication::Initialize(std::string & _Error)
{
  return m_Editor->Initialize(_Error);
}

void CApplication::locateResources()
{
  OgreBites::ApplicationContext::locateResources();

  if (!m_Editor->GetAssetManager()->MountResources())
    m_Editor->GetLog()->Write(core::ELogLevel::Error, "Could not mount project Assets folder.");
}

void CApplication::setup()
{
  OgreBites::ApplicationContext::setup();

  m_SceneManager = getRoot()->createSceneManager("DefaultSceneManager", "EditorScene");
  Ogre::RTShader::ShaderGenerator::getSingleton().addSceneManager(m_SceneManager);
  m_SceneManager->addRenderQueueListener(getOverlaySystem());

  auto * Camera = m_SceneManager->createCamera("EditorCamera");
  Camera->setNearClipDistance(0.1f);
  Camera->setAutoAspectRatio(true);

  auto * CameraNode = m_SceneManager->getRootSceneNode()->createChildSceneNode();
  CameraNode->setPosition(0.0f, 0.0f, 5.0f);
  CameraNode->lookAt(Ogre::Vector3::ZERO, Ogre::Node::TS_WORLD);
  CameraNode->attachObject(Camera);

  auto * Viewport = getRenderWindow()->addViewport(Camera);
  Viewport->setBackgroundColour(Ogre::ColourValue(0.12f, 0.13f, 0.15f));

  Ogre::OverlayManager::getSingleton().setPixelRatio(getDisplayDPI() / 96.0f);

  initialiseImGui()->show();

  addInputListener(getImGuiInputListener());
  addInputListener(this);

  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#if defined(IMGUI_HAS_DOCK)
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif
}

void CApplication::shutdown()
{
  if (m_SceneManager)
  {
    if (auto * ShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr())
      ShaderGenerator->removeSceneManager(m_SceneManager);

    m_SceneManager = nullptr;
  }

  if (m_Editor)
    m_Editor->Shutdown();

  OgreBites::ApplicationContext::shutdown();
}

bool CApplication::frameStarted(const Ogre::FrameEvent & _Event)
{
  if (!OgreBites::ApplicationContext::frameStarted(_Event))
    return false;

  m_Editor->Tick(_Event.timeSinceLastFrame);

  Ogre::ImGuiOverlay::NewFrame();

  m_Editor->Draw();

  return true;
}

bool CApplication::keyPressed(const OgreBites::KeyboardEvent & _Event)
{
  if (_Event.keysym.sym == OgreBites::SDLK_F5)
  {
    m_Editor->GetAssetManager()->Refresh();
    return true;
  }

  if (_Event.keysym.sym == OgreBites::SDLK_ESCAPE)
  {
    getRoot()->queueEndRendering();
    return true;
  }

  return false;
}

} // namespace editor::platform
