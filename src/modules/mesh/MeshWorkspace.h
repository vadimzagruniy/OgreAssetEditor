#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

class CMeshInstance;

class CMeshWorkspace final:
  public CEditorWorkspace
{
public: // Constructor

  explicit CMeshWorkspace(std::unique_ptr<CMeshInstance> _Instance);

public: // CEditorWorkspace

  void OnGui() override;
};

} // namespace editor
