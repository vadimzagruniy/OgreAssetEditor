#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <cstdint>
#include <memory>
#include <string>

namespace editor
{

class CTextDocument;

class CTextWorkspace final:
  public CEditorWorkspace
{
public: // Constructor

  explicit CTextWorkspace(std::shared_ptr<CTextDocument> _Document);

public: // CEditorWorkspace

  void OnGui() override;

private: // Implementation

  void SyncBuffer();

  void CommitBuffer();

  void HandleHistoryShortcuts();

private: // Attributes

  std::shared_ptr<CTextDocument> m_Document;
  std::string                    m_Buffer;
  std::uint64_t                  m_BufferRevision;
  unsigned int                   m_EditSession;
};

} // namespace editor
