#pragma once

#include "editor/workspaces/EditorWorkspace.h"
#include "ui/layout/Layout.h"

#include <OgreVector.h>

#include <memory>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

namespace ui
{
class CComboBox;
} // namespace ui

class CRenderScene;
class CSceneDocument;
class CSceneEditorController;
class CSceneInstance;
class CSceneSync;
class CTransformGizmo;
class CViewport;
struct TViewportFrame;

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

  void SetupViewport();

  void SetupToolbar();

  void DrawToolbar();

  void HandleShortcuts();

  bool DrawOverlay(const TViewportFrame & _Frame);

  void HandleAssetDrop(const TViewportFrame & _Frame);

  Ogre::Vector3 FindDropPosition(const TViewportFrame & _Frame) const;

  bool DrawGizmo(const TViewportFrame & _Frame);

private: // Attributes

  std::shared_ptr<CSceneEditorController> m_Controller;
  std::shared_ptr<CSceneDocument>         m_Document;
  std::unique_ptr<CRenderScene>           m_RenderScene;
  std::unique_ptr<CSceneSync>             m_Sync;
  std::unique_ptr<CViewport>              m_Viewport;
  std::unique_ptr<CTransformGizmo>        m_Gizmo;
  ui::CLayout                             m_Toolbar;
  ui::CComboBox &                         m_ModeCombo;
  ui::CComboBox &                         m_SpaceCombo;
};

} // namespace editor
