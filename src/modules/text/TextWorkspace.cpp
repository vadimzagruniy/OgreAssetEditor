#include "TextWorkspace.h"
#include "TextInstance.h"

#include <imgui.h>

#include <string>
#include <utility>

namespace editor
{

namespace
{

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

} // namespace

//
// Constructor
//

CTextWorkspace::CTextWorkspace(std::unique_ptr<CTextInstance> _Instance):
  CEditorWorkspace(std::move(_Instance))
{
  // Empty
}

//
// CEditorWorkspace
//

bool CTextWorkspace::IsDirty() const
{
  return static_cast<const CTextInstance *>(GetInstance())->IsDirty();
}

void CTextWorkspace::OnGui()
{
  auto * Instance = static_cast<CTextInstance *>(GetInstance());

  auto &     Content    = Instance->GetContent();
  const auto Available  = ImGui::GetContentRegionAvail();
  const auto InputFlags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize;

  if (ImGui::InputTextMultiline("##Text", Content.data(), Content.capacity() + 1, Available, InputFlags, ResizeStringCallback, &Content))
    Instance->MarkDirty();
}

//
// Protected CEditorWorkspace
//

bool CTextWorkspace::OnSave(std::string & _Error)
{
  auto & Instance = *static_cast<CTextInstance *>(GetInstance());

  if (Instance.Save())
    return true;

  _Error = "Could not write " + GetTitle();

  return false;
}

} // namespace editor
