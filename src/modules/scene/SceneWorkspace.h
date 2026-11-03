#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

class CSceneInstance;

class CSceneWorkspace final:
  public CEditorWorkspace
{
public: // Constructor/Destructor

  explicit CSceneWorkspace(std::unique_ptr<CSceneInstance> _Instance);

  ~CSceneWorkspace() override;

public: // CEditorWorkspace

  void OnGui() override;

protected: // CEditorWorkspace

  bool OnSave(std::string & _Error) override;

private: // Implementation

  CSceneInstance & GetScene() const;
};

} // namespace editor
