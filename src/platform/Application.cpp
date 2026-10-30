#include "Application.h"
#include "core/Log.h"
#include "editor/Editor.h"
#include "editor/assets/AssetManager.h"
#include "ui/EditorStyle.h"

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

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include <windows.h>
#endif

namespace editor::platform
{

namespace
{

constexpr uint32_t DefaultWindowWidth  = 1600;
constexpr uint32_t DefaultWindowHeight = 900;

} // namespace

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

  MaximizeWindow();

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

  const float UiScale = getDisplayDPI() / 96.0f;
  Ogre::OverlayManager::getSingleton().setPixelRatio(UiScale);

  auto & Overlay = *initialiseImGui();

  if (!ui::LoadEditorFont(getFSLayer().getConfigFilePath("fonts/Roboto-Medium.ttf"), UiScale))
    m_Editor->GetLog()->Write(core::ELogLevel::Warning, "Editor font not found, using the default ImGui font.");

  ui::ApplyEditorStyle(UiScale);
  Overlay.show();

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

OgreBites::NativeWindowPair CApplication::createWindow(const Ogre::String & _Name, uint32_t _Width, uint32_t _Height, Ogre::NameValuePairList _MiscParams)
{
  getRoot()->getRenderSystem()->setConfigOption("Full Screen", "No");

  const uint32_t Width  = _Width > 0 ? _Width : DefaultWindowWidth;
  const uint32_t Height = _Height > 0 ? _Height : DefaultWindowHeight;

  return OgreBites::ApplicationContext::createWindow(_Name, Width, Height, std::move(_MiscParams));
}

//
// Implementation
//

void CApplication::MaximizeWindow()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  HWND WindowHandle = nullptr;
  getRenderWindow()->getCustomAttribute("WINDOW", &WindowHandle);

  if (WindowHandle)
    ShowWindow(WindowHandle, SW_MAXIMIZE);
#endif
}

} // namespace editor::platform
