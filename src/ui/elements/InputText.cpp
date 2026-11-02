#include "InputText.h"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace editor::ui
{

namespace
{

int ResizeTextBuffer(ImGuiInputTextCallbackData * _Data)
{
  if (_Data->EventFlag != ImGuiInputTextFlags_CallbackResize)
    return 0;

  auto & Text = *static_cast<std::string *>(_Data->UserData);
  Text.resize(static_cast<std::size_t>(_Data->BufTextLen));
  _Data->Buf = Text.data();

  return 0;
}

} // namespace

//
// Constructor
//

CInputText::CInputText(std::string _Label):
  CElementSetup(std::move(_Label))
{
  // Empty
}

//
// CElement
//

ImVec2 CInputText::CalcContentSize() const
{
  const float TextWidth  = std::max(ImGui::CalcTextSize(m_Text.c_str()).x, ImGui::CalcTextSize(m_Hint.c_str()).x);
  const float FrameWidth = TextWidth + ImGui::GetStyle().FramePadding.x * 2.0f;

  return ImVec2(FrameWidth + CalcSideLabelWidth(), ImGui::GetFrameHeight());
}

void CInputText::OnDraw(const ImVec2 & _Size)
{
  ImGui::SetNextItemWidth(std::max(_Size.x - CalcSideLabelWidth(), 1.0f));

  const bool IsChanged = ImGui::InputTextWithHint(GetLabel().c_str(), m_Hint.c_str(), m_Text.data(), m_Text.capacity() + 1,
                                                  ImGuiInputTextFlags_CallbackResize, ResizeTextBuffer, &m_Text);

  if (IsChanged && m_ChangeCallback)
    m_ChangeCallback(m_Text);
}

//
// Setup
//

CInputText & CInputText::SetText(std::string _Text)
{
  m_Text = std::move(_Text);
  return *this;
}

CInputText & CInputText::SetHint(std::string _Hint)
{
  m_Hint = std::move(_Hint);
  return *this;
}

CInputText & CInputText::OnChanged(std::function<void(const std::string &)> _Callback)
{
  m_ChangeCallback = std::move(_Callback);
  return *this;
}

//
// Interface
//

const std::string & CInputText::GetText() const noexcept
{
  return m_Text;
}

} // namespace editor::ui
