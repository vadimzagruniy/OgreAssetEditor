#include "CheckBox.h"

#include <utility>

namespace editor::ui
{

//
// Constructor
//

CCheckBox::CCheckBox(std::string _Label):
  CElementSetup(std::move(_Label)),
  m_IsChecked(false)
{
  // Empty
}

//
// CElement
//

ImVec2 CCheckBox::CalcContentSize() const
{
  const float FrameHeight = ImGui::GetFrameHeight();

  return ImVec2(FrameHeight + CalcSideLabelWidth(), FrameHeight);
}

void CCheckBox::OnDraw(const ImVec2 & /*_Size*/)
{
  if (!ImGui::Checkbox(GetLabel().c_str(), &m_IsChecked))
    return;

  if (m_ToggleCallback)
    m_ToggleCallback(m_IsChecked);
}

//
// Setup
//

CCheckBox & CCheckBox::SetChecked(bool _IsChecked) noexcept
{
  m_IsChecked = _IsChecked;
  return *this;
}

CCheckBox & CCheckBox::OnToggled(std::function<void(bool)> _Callback)
{
  m_ToggleCallback = std::move(_Callback);
  return *this;
}

//
// Interface
//

bool CCheckBox::IsChecked() const noexcept
{
  return m_IsChecked;
}

} // namespace editor::ui
