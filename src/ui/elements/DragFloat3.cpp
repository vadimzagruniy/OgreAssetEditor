#include "DragFloat3.h"

#include <algorithm>
#include <utility>

namespace editor::ui
{

//
// Constructor
//

CDragFloat3::CDragFloat3(std::string _Label):
  CElementSetup(std::move(_Label)),
  m_Format("%.3f"),
  m_Value{0.0f, 0.0f, 0.0f},
  m_Minimum(0.0f),
  m_Maximum(0.0f),
  m_Speed(1.0f)
{
  // Empty
}

//
// CElement
//

ImVec2 CDragFloat3::CalcContentSize() const
{
  return CalcDefaultItemSize();
}

float CDragFloat3::CalcLabelExtent() const
{
  return CalcSideLabelWidth();
}

void CDragFloat3::OnDraw(const ImVec2 & _Size)
{
  ImGui::SetNextItemWidth(std::max(_Size.x - CalcLabelExtent(), 1.0f));

  const ImGuiSliderFlags Flags = m_Minimum < m_Maximum ? ImGuiSliderFlags_AlwaysClamp : ImGuiSliderFlags_None;
  if (!ImGui::DragFloat3(GetLabel().c_str(), m_Value.data(), m_Speed, m_Minimum, m_Maximum, m_Format.c_str(), Flags))
    return;

  if (m_ChangeCallback)
    m_ChangeCallback(m_Value);
}

//
// Setup
//

CDragFloat3 & CDragFloat3::SetValue(const TValue & _Value) noexcept
{
  m_Value = _Value;
  return *this;
}

CDragFloat3 & CDragFloat3::SetRange(float _Minimum, float _Maximum) noexcept
{
  m_Minimum = _Minimum;
  m_Maximum = _Maximum;
  return *this;
}

CDragFloat3 & CDragFloat3::SetSpeed(float _Speed) noexcept
{
  m_Speed = _Speed;
  return *this;
}

CDragFloat3 & CDragFloat3::SetFormat(std::string _Format)
{
  m_Format = std::move(_Format);
  return *this;
}

CDragFloat3 & CDragFloat3::OnChanged(std::function<void(const TValue &)> _Callback)
{
  m_ChangeCallback = std::move(_Callback);
  return *this;
}

//
// Interface
//

const CDragFloat3::TValue & CDragFloat3::GetValue() const noexcept
{
  return m_Value;
}

} // namespace editor::ui
