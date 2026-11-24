#include "TextWorkspace.h"
#include "TextChange.h"
#include "TextDocument.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>

namespace editor
{

namespace
{

constexpr const char * TextInputId = "##Text";

int ResizeStringCallback(ImGuiInputTextCallbackData * _Data)
{
  if (_Data->EventFlag == ImGuiInputTextFlags_CallbackResize)
  {
    auto * Text = static_cast<std::string *>(_Data->UserData);
    Text->resize(static_cast<std::size_t>(_Data->BufTextLen));
    _Data->Buf = Text->data();
  }

  return 0;
}

std::optional<TTextChange> FindChange(const std::string & _Before, const std::string & _After)
{
  const std::size_t CommonLength = std::min(_Before.size(), _After.size());

  std::size_t Prefix = 0;
  while (Prefix < CommonLength && _Before[Prefix] == _After[Prefix])
    ++Prefix;

  if (Prefix == _Before.size() && Prefix == _After.size())
    return std::nullopt;

  std::size_t Suffix = 0;
  while (Suffix < CommonLength - Prefix && _Before[_Before.size() - 1 - Suffix] == _After[_After.size() - 1 - Suffix])
    ++Suffix;

  return TTextChange{Prefix,
                     _Before.substr(Prefix, _Before.size() - Prefix - Suffix),
                     _After.substr(Prefix, _After.size() - Prefix - Suffix)};
}

} // namespace

//
// Constructor
//

CTextWorkspace::CTextWorkspace(std::shared_ptr<CTextDocument> _Document):
  CEditorWorkspace(_Document),
  m_Document(std::move(_Document)),
  m_Buffer(m_Document->GetContent()),
  m_BufferRevision(m_Document->GetRevision()),
  m_EditSession(0)
{
  // Empty
}

//
// CEditorWorkspace
//

void CTextWorkspace::OnGui()
{
  SyncBuffer();

  const auto Available  = ImGui::GetContentRegionAvail();
  const auto InputFlags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_NoUndoRedo;

  const bool IsChanged = ImGui::InputTextMultiline(TextInputId, m_Buffer.data(), m_Buffer.capacity() + 1, Available, InputFlags,
                                                   ResizeStringCallback, &m_Buffer);

  if (ImGui::IsItemActivated())
    ++m_EditSession;

  if (IsChanged)
    CommitBuffer();

  if (ImGui::IsItemActive())
    HandleHistoryShortcuts();
}

//
// Implementation
//

void CTextWorkspace::SyncBuffer()
{
  if (m_Document->GetRevision() == m_BufferRevision)
    return;

  m_Buffer         = m_Document->GetContent();
  m_BufferRevision = m_Document->GetRevision();

  auto * InputState = ImGui::GetInputTextState(ImGui::GetID(TextInputId));
  if (!InputState)
    return;

  // While the field is active ImGui edits its own copy of the text, so it has to re-read the buffer.
  // The cursor is placed after the undone or redone change; ImGui counts it in code points.
  const char * Text   = m_Buffer.data();
  const int    Cursor = ImTextCountCharsFromUtf8(Text, Text + m_Document->GetLastChangeEnd());

  InputState->ReloadUserBuf        = true;
  InputState->ReloadSelectionStart = Cursor;
  InputState->ReloadSelectionEnd   = Cursor;
}

void CTextWorkspace::CommitBuffer()
{
  auto Change = FindChange(m_Document->GetContent(), m_Buffer);
  if (!Change)
    return;

  m_Document->Edit(std::move(*Change), m_EditSession);

  m_BufferRevision = m_Document->GetRevision();
}

void CTextWorkspace::HandleHistoryShortcuts()
{
  // The active text field owns the Ctrl+Z / Ctrl+Y routes, so the editor's global shortcuts do not fire.
  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z))
    m_Document->GetHistory().Undo();

  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y) || ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z))
    m_Document->GetHistory().Redo();
}

} // namespace editor
