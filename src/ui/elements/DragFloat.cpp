#include "DragFloat.h"

#include <algorithm>
#include <utility>

namespace editor::ui
{

//
// Constructor
//

CDragFloat::CDragFloat(std::string _Label):
  CElementSetup(std::move(_Label)),
  m_Format("%.3f"),
  m_Value(0.0f),
  m_Minimum(0.0f),
  m_Maximum(0.0f),
  m_Speed(1.0f)
{
  // Empty
}

//
// CElement
//

ImVec2 CDragFloat::CalcContentSize() const
{
  return CalcDefaultItemSize();
}

float CDragFloat::CalcLabelExtent() const
{
  return CalcSideLabelWidth();
}

void CDragFloat::OnDraw(const ImVec2 & _Size)
{
  ImGui::SetNextItemWidth(std::max(_Size.x - CalcLabelExtent(), 1.0f));

  const ImGuiSliderFlags Flags = m_Minimum < m_Maximum ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None;
  if (!ImGui::DragFloat(GetLabel().c_str(), &m_Value, m_Speed, m_Minimum, m_Maximum, m_Format.c_str(), Flags))
    return;

  if (m_ChangeCallback)
    m_ChangeCallback(m_Value);
}

//
// Setup
//

CDragFloat & CDragFloat::SetValue(float _Value) noexcept
{
  m_Value = _Value;
  return *this;
}

CDragFloat & CDragFloat::SetRange(float _Minimum, float _Maximum) noexcept
{
  m_Minimum = _Minimum;
  m_Maximum = _Maximum;
  return *this;
}

CDragFloat & CDragFloat::SetSpeed(float _Speed) noexcept
{
  m_Speed = _Speed;
  return *this;
}

CDragFloat & CDragFloat::SetFormat(std::string _Format)
{
  m_Format = std::move(_Format);
  return *this;
}

CDragFloat & CDragFloat::OnChanged(std::function<void(float)> _Callback)
{
  m_ChangeCallback = std::move(_Callback);
  return *this;
}

//
// Interface
//

float CDragFloat::GetValue() const noexcept
{
  return m_Value;
}

} // namespace editor::ui
