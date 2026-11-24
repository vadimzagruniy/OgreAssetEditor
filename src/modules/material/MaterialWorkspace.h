#pragma once

#include "editor/workspaces/EditorWorkspace.h"
#include "ui/layout/Layout.h"

#include <cstddef>
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
class CMaterialEditorController;
class CMaterialSelection;
class CPassPropertySource;
class CPreviewScene;
class CTextureUnitPropertySource;
class CViewport;
struct TMaterialNode;

class CMaterialWorkspace final:
  public CEditorWorkspace
{
public: // Constructor/Destructor

  CMaterialWorkspace(std::shared_ptr<CMaterialDocument> _Document, std::shared_ptr<CMaterialEditorController> _Controller);

  ~CMaterialWorkspace() override;

public: // CEditorWorkspace

  std::vector<core::TTypeKey> GetToolBoxes() const override;

  std::shared_ptr<IPropertySource> GetInspected() const override;

  void OnActivated() override;

  void OnDeactivated() override;

  void OnGui() override;

private: // Implementation

  void SetupToolbar();

  void DrawToolbar();

  void UpdatePreviewMaterial();

private: // Events

  void OnDocumentChanged(const TMaterialNode & _Node);

private: // Attributes

  std::shared_ptr<CMaterialDocument>          m_Document;
  std::shared_ptr<CMaterialSelection>         m_Selection;
  std::shared_ptr<CMaterialEditorController>  m_Controller;
  std::shared_ptr<CPassPropertySource>        m_PassProperties;
  std::shared_ptr<CTextureUnitPropertySource> m_TextureUnitProperties;
  std::unique_ptr<CPreviewScene>              m_Preview;
  std::unique_ptr<CViewport>                  m_Viewport;
  EPreviewShape                               m_Shape;
  int                                         m_PreviewMaterialIndex;
  ui::CLayout                                 m_Toolbar;
  ui::CComboBox &                             m_ShapeCombo;
  ui::CText &                                 m_WarningText;
  std::size_t                                 m_ChangedConnection;
};

} // namespace editor
