#include "ColorEdit.h"

#include <algorithm>
#include <utility>

namespace editor::ui
{

//
// Constructor
//

CColorEdit::CColorEdit(std::string _Label):
  CElementSetup(std::move(_Label)),
  m_Value(1.0f, 1.0f, 1.0f, 1.0f),
  m_IsAlphaEnabled(true)
{
  // Empty
}

//
// CElement
//

ImVec2 CColorEdit::CalcContentSize() const
{
  return CalcDefaultItemSize();
}

float CColorEdit::CalcLabelExtent() const
{
  return CalcSideLabelWidth();
}

void CColorEdit::OnDraw(const ImVec2 & _Size)
{
  ImGui::SetNextItemWidth(std::max(_Size.x - CalcLabelExtent(), 1.0f));

  const ImGuiColorEditFlags Flags = m_IsAlphaEnabled ? ImGuiColorEditFlags_None : ImGuiColorEditFlags_NoAlpha;
  if (!ImGui::ColorEdit4(GetLabel().c_str(), &m_Value.x, Flags))
    return;

  if (m_ChangeCallback)
    m_ChangeCallback(m_Value);
}

//
// Setup
//

CColorEdit & CColorEdit::SetValue(const ImVec4 & _Value) noexcept
{
  m_Value = _Value;
  return *this;
}

CColorEdit & CColorEdit::SetAlphaEnabled(bool _IsAlphaEnabled) noexcept
{
  m_IsAlphaEnabled = _IsAlphaEnabled;
  return *this;
}

CColorEdit & CColorEdit::OnChanged(std::function<void(const ImVec4 &)> _Callback)
{
  m_ChangeCallback = std::move(_Callback);
  return *this;
}

//
// Interface
//

const ImVec4 & CColorEdit::GetValue() const noexcept
{
  return m_Value;
}

} // namespace editor::ui
