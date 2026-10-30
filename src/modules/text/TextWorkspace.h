#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>
#include <string>

namespace editor
{

class CTextInstance;

class CTextWorkspace final:
  public CEditorWorkspace
{
public: // Constructor

  explicit CTextWorkspace(std::unique_ptr<CTextInstance> _Instance);

public: // CEditorWorkspace

  bool IsDirty() const override;

  void OnGui() override;

protected: // CEditorWorkspace

  bool OnSave(std::string & _Error) override;
};

} // namespace editor
