#include "Button.h"

#include <utility>

namespace editor::ui
{

//
// Constructor
//

CButton::CButton(std::string _Label):
  CElementSetup(std::move(_Label))
{
  // Empty
}

//
// CElement
//

ImVec2 CButton::CalcContentSize() const
{
  return CalcLabelFrameSize();
}

void CButton::OnDraw(const ImVec2 & _Size)
{
  if (ImGui::Button(GetLabel().c_str(), _Size) && m_ClickCallback)
    m_ClickCallback();
}

//
// Setup
//

CButton & CButton::OnClick(std::function<void()> _Callback)
{
  m_ClickCallback = std::move(_Callback);
  return *this;
}

} // namespace editor::ui
