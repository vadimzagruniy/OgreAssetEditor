#pragma once

#include "editor/workspaces/EditorWorkspace.h"
#include "ui/layout/Layout.h"

#include <OgreVector.h>

#include <cstddef>
#include <memory>
#include <string>

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
class CSceneAnimationHost;
class CSceneDocument;
class CSceneEditorController;
class CSceneNodeBindings;
class CSceneNodePropertySource;
class CSceneSelection;
class CSceneSync;
class CSceneViewportIcons;
class CTransformGizmo;
class CViewport;
struct TSceneChange;
struct TViewportFrame;

class CSceneWorkspace final:
  public CEditorWorkspace
{
public: // Constructor/Destructor

  CSceneWorkspace(std::shared_ptr<CSceneDocument> _Document, std::shared_ptr<CSceneEditorController> _Controller, std::string _ResourceGroup, std::shared_ptr<const CSceneNodeBindings> _NodeBindings, std::shared_ptr<core::CLog> _Log);

  ~CSceneWorkspace() override;

public: // CEditorWorkspace

  std::vector<core::TTypeKey> GetToolBoxes() const override;

  std::shared_ptr<IPropertySource> GetInspected() const override;

  void OnActivated() override;

  void OnDeactivated() override;

  void OnGui() override;

private: // Implementation

  void SetupViewport();

  void SetupToolbar();

  void DrawToolbar();

  void HandleShortcuts();

  void HandleEditShortcuts();

  bool DrawOverlay(const TViewportFrame & _Frame);

  void HandleAssetDrop(const TViewportFrame & _Frame);

  void HandleTemplateDrop(const TViewportFrame & _Frame);

  Ogre::Vector3 FindDropPosition(const TViewportFrame & _Frame) const;

  bool DrawGizmo(const TViewportFrame & _Frame);

  void UpdateHeadlight();

private: // Events

  void OnDocumentChanged(const TSceneChange & _Change);

private: // Attributes

  std::shared_ptr<CSceneDocument>           m_Document;
  std::shared_ptr<CSceneSelection>          m_Selection;
  std::shared_ptr<CSceneEditorController>   m_Controller;
  std::shared_ptr<CSceneNodePropertySource> m_NodeProperties;
  std::shared_ptr<CSceneAnimationHost>      m_AnimationHost;
  std::unique_ptr<CRenderScene>             m_RenderScene;
  std::unique_ptr<CSceneSync>               m_Sync;
  std::unique_ptr<CViewport>                m_Viewport;
  std::unique_ptr<CTransformGizmo>          m_Gizmo;
  std::unique_ptr<CSceneViewportIcons>      m_Icons;
  ui::CLayout                               m_Toolbar;
  ui::CComboBox &                           m_ModeCombo;
  ui::CComboBox &                           m_SpaceCombo;
  std::size_t                               m_ChangedConnection;
};

} // namespace editor
