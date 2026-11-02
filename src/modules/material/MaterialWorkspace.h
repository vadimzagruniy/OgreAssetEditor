#pragma once

#include "editor/workspaces/EditorWorkspace.h"
#include "ui/layout/Layout.h"

#include <memory>

namespace editor
{

namespace ui
{
class CComboBox;
class CText;
} // namespace ui

enum class EPreviewShape;
class CMaterialDocument;
class CMaterialEditingSession;
class CMaterialInstance;
class CPreviewScene;

class CMaterialWorkspace final:
  public CEditorWorkspace
{
public: // Constructor/Destructor

  CMaterialWorkspace(std::unique_ptr<CMaterialInstance> _Instance, std::shared_ptr<CMaterialEditingSession> _Session);

  ~CMaterialWorkspace() override;

public: // CEditorWorkspace

  std::vector<core::TTypeKey> GetToolBoxes() const override;

  void OnActivated() override;

  void OnDeactivated() override;

  void OnGui() override;

protected: // CEditorWorkspace

  bool OnSave(std::string & _Error) override;

private: // Implementation

  void SetupToolbar();

  void DrawToolbar();

  void UpdatePreviewMaterial();

private: // Attributes

  std::shared_ptr<CMaterialEditingSession> m_Session;
  std::shared_ptr<CMaterialDocument>       m_Document;
  std::unique_ptr<CPreviewScene>           m_Preview;
  EPreviewShape                            m_Shape;
  int                                      m_PreviewMaterialIndex;
  ui::CLayout                              m_Toolbar;
  ui::CComboBox &                          m_ShapeCombo;
  ui::CText &                              m_WarningText;
};

} // namespace editor
