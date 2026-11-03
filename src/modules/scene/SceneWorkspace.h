#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

class CRenderScene;
class CSceneDocument;
class CSceneEditorController;
class CSceneInstance;
class CSceneSync;
class CViewport;

class CSceneWorkspace final:
  public CEditorWorkspace
{
public: // Constructor/Destructor

  CSceneWorkspace(std::unique_ptr<CSceneInstance> _Instance, std::shared_ptr<CSceneEditorController> _Controller, std::shared_ptr<core::CLog> _Log);

  ~CSceneWorkspace() override;

public: // CEditorWorkspace

  std::vector<core::TTypeKey> GetToolBoxes() const override;

  void OnActivated() override;

  void OnDeactivated() override;

  void OnGui() override;

protected: // CEditorWorkspace

  bool OnSave(std::string & _Error) override;

private: // Implementation

  CSceneInstance & GetScene() const;

private: // Attributes

  std::shared_ptr<CSceneEditorController> m_Controller;
  std::shared_ptr<CSceneDocument>         m_Document;
  std::unique_ptr<CRenderScene>           m_RenderScene;
  std::unique_ptr<CSceneSync>             m_Sync;
  std::unique_ptr<CViewport>              m_Viewport;
};

} // namespace editor
