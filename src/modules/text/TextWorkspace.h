#pragma once

#include "editor/workspaces/EditorWorkspace.h"

#include <cstdint>
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

  void OnGui() override;

protected: // CEditorWorkspace

  bool OnSave(std::string & _Error) override;

private: // Implementation

  CTextInstance & GetTextInstance() const noexcept;

  void SyncBuffer();

  void CommitBuffer();

  void HandleHistoryShortcuts();

private: // Attributes

  std::string   m_Buffer;
  std::uint64_t m_BufferRevision;
  unsigned int  m_EditSession;
};

} // namespace editor
