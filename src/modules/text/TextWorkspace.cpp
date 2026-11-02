#include "TextWorkspace.h"
#include "TextChange.h"
#include "TextEditCommand.h"
#include "TextInstance.h"

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

CTextWorkspace::CTextWorkspace(std::unique_ptr<CTextInstance> _Instance):
  CEditorWorkspace(std::move(_Instance)),
  m_Buffer(GetTextInstance().GetContent()),
  m_BufferRevision(GetTextInstance().GetRevision()),
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
// Protected CEditorWorkspace
//

bool CTextWorkspace::OnSave(std::string & _Error)
{
  if (GetTextInstance().Save())
    return true;

  _Error = "Could not write " + GetTitle();

  return false;
}

//
// Implementation
//

CTextInstance & CTextWorkspace::GetTextInstance() const noexcept
{
  return *static_cast<CTextInstance *>(GetInstance());
}

void CTextWorkspace::SyncBuffer()
{
  const auto & Instance = GetTextInstance();
  if (Instance.GetRevision() == m_BufferRevision)
    return;

  m_Buffer         = Instance.GetContent();
  m_BufferRevision = Instance.GetRevision();

  auto * InputState = ImGui::GetInputTextState(ImGui::GetID(TextInputId));
  if (!InputState)
    return;

  // While the field is active ImGui edits its own copy of the text, so it has to re-read the buffer.
  // The cursor is placed after the undone or redone change; ImGui counts it in code points.
  const char * Text   = m_Buffer.data();
  const int    Cursor = ImTextCountCharsFromUtf8(Text, Text + Instance.GetLastChangeEnd());

  InputState->ReloadUserBuf        = true;
  InputState->ReloadSelectionStart = Cursor;
  InputState->ReloadSelectionEnd   = Cursor;
}

void CTextWorkspace::CommitBuffer()
{
  auto & Instance = GetTextInstance();

  auto Change = FindChange(Instance.GetContent(), m_Buffer);
  if (!Change)
    return;

  GetUndoStack().Execute(std::make_unique<CTextEditCommand>(Instance, std::move(*Change), m_EditSession));

  m_BufferRevision = Instance.GetRevision();
}

void CTextWorkspace::HandleHistoryShortcuts()
{
  // The active text field owns the Ctrl+Z / Ctrl+Y routes, so the editor's global shortcuts do not fire.
  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z))
    GetUndoStack().Undo();

  if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y) || ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z))
    GetUndoStack().Redo();
}

} // namespace editor
