#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

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

  void DrawToolbar();

  void UpdatePreviewMaterial();

private: // Attributes

  std::shared_ptr<CMaterialEditingSession> m_Session;
  std::shared_ptr<CMaterialDocument>       m_Document;
  std::unique_ptr<CPreviewScene>           m_Preview;
  EPreviewShape                            m_Shape;
  int                                      m_PreviewMaterialIndex;
};

} // namespace editor
