#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <memory>

namespace editor
{

class CTextureDocument;

class CTextureWorkspace final:
  public CEditorWorkspace
{
public: // Constructor

  explicit CTextureWorkspace(std::shared_ptr<CTextureDocument> _Document);

public: // CEditorWorkspace

  void OnGui() override;

private: // Attributes

  std::shared_ptr<CTextureDocument> m_Document;
};

} // namespace editor
