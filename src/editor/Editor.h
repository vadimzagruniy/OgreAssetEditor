#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace editor
{

namespace core
{
class CEditorService;
class CLog;
} // namespace core

namespace ui
{
class CWindowManager;
} // namespace ui

class CAssetManager;
class CAssetTypeRegistry;
class CModulesManager;
class CWorkspaceManager;

class CEditor
{
public: // Constructor/Destructor

  explicit CEditor(std::filesystem::path _ProjectRoot);

  ~CEditor();

public: // Interface

  bool Initialize(std::string & _Error);

  void Shutdown() noexcept;

  void Tick(float _DeltaTime);

  void Draw();

  void SetQuitCallback(std::function<void()> _Callback);

public: // Services

  std::shared_ptr<CAssetManager> GetAssetManager() const noexcept;

  std::shared_ptr<core::CLog> GetLog() const noexcept;

private: // Implementation

  void DrawMainMenu();

  void DrawDockSpace();

  void BuildDefaultLayout(unsigned int _DockSpaceId);

private: // Attributes

  std::shared_ptr<core::CLog>           m_Log;
  std::shared_ptr<CAssetManager>        m_AssetManager;
  std::shared_ptr<CAssetTypeRegistry>   m_AssetTypes;
  std::shared_ptr<CWorkspaceManager>    m_WorkspaceManager;
  std::shared_ptr<ui::CWindowManager>   m_WindowManager;
  std::unique_ptr<core::CEditorService> m_Services;
  std::unique_ptr<CModulesManager>      m_Modules;
  std::function<void()>                 m_RequestQuit;
  bool                                  m_IsLayoutChecked;
  bool                                  m_ResetLayoutRequested;
};

} // namespace editor
