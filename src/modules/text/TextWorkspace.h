#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

class CTextInstance;

class CTextWorkspace final:
  public CEditorWorkspace
{
public: // Constructor

  explicit CTextWorkspace(std::unique_ptr<CTextInstance> _Instance);

public: // CEditorWorkspace

  void OnGui() override;
};

} // namespace editor
