#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

class CMeshDocument;
class CPreviewScene;
class CViewport;

class CMeshWorkspace final:
  public CEditorWorkspace
{
public: // Constructor/Destructor

  explicit CMeshWorkspace(const std::shared_ptr<CMeshDocument> & _Document);

  ~CMeshWorkspace() override;

public: // CEditorWorkspace

  void OnActivated() override;

  void OnDeactivated() override;

  void OnGui() override;

private: // Attributes

  std::unique_ptr<CPreviewScene> m_Preview;
  std::unique_ptr<CViewport>     m_Viewport;
};

} // namespace editor
