#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

class CMeshInstance;
class CMeshPreview;

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

private: // Implementation

  void HandleViewportInput();

private: // Attributes

  std::unique_ptr<CMeshPreview> m_Preview;
};

} // namespace editor
