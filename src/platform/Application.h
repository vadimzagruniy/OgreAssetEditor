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
  public OgreBites::ApplicationContext
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

  OgreBites::NativeWindowPair createWindow(const Ogre::String & _Name, uint32_t _Width, uint32_t _Height, Ogre::NameValuePairList _MiscParams) override;

private: // Implementation

  void MaximizeWindow();

private: // Attributes

  std::unique_ptr<CEditor> m_Editor;
  Ogre::SceneManager *     m_SceneManager = nullptr;
};

} // namespace editor::platform
