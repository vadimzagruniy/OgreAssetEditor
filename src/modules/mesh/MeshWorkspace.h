#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

class CMeshInstance;
class CPreviewScene;

class CMeshWorkspace final:
  public CEditorWorkspace
{
public: // Constructor/Destructor

  explicit CMeshWorkspace(std::unique_ptr<CMeshInstance> _Instance);

  ~CMeshWorkspace() override;

public: // CEditorWorkspace

  void OnActivated() override;

  void OnDeactivated() override;

  void OnGui() override;

private: // Attributes

  std::unique_ptr<CPreviewScene> m_Preview;
};

} // namespace editor
