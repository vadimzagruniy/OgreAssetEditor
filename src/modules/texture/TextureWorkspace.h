#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

class CTextureInstance;

class CTextureWorkspace final:
  public CEditorWorkspace
{
public: // Constructor

  explicit CTextureWorkspace(std::unique_ptr<CTextureInstance> _Instance);

public: // CEditorWorkspace

  void OnGui() override;
};

} // namespace editor
