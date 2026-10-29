#pragma once

#include <OgreApplicationContext.h>

#include <filesystem>
#include <memory>
#include <string>

namespace Ogre
{
class SceneManager;
} // namespace Ogre

namespace editor
{
class CEditor;
} // namespace editor

namespace editor::platform
{

class CApplication final:
  public OgreBites::ApplicationContext,
  public OgreBites::InputListener
{
public: // Constructor/Destructor

  explicit CApplication(std::filesystem::path _ProjectRoot);

  ~CApplication() override;

public: // ApplicationContext

  bool Initialize(std::string & _Error);

  void locateResources() override;

  void setup() override;

  void shutdown() override;

  bool frameStarted(const Ogre::FrameEvent & _Event) override;

  bool keyPressed(const OgreBites::KeyboardEvent & _Event) override;

private: // Attributes

  std::unique_ptr<CEditor> m_Editor;
  Ogre::SceneManager *     m_SceneManager = nullptr;
};

} // namespace editor::platform
