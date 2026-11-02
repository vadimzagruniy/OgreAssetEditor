#include "ArrowButton.h"

#include <utility>

namespace editor::ui
{

//
// Constructor
//

CArrowButton::CArrowButton(std::string _Label, ImGuiDir _Direction):
  CElementSetup(std::move(_Label)),
  m_Direction(_Direction)
{
  // Empty
}

//
// CElement
//

ImVec2 CArrowButton::CalcContentSize() const
{
  const float FrameHeight = ImGui::GetFrameHeight();

  return ImVec2(FrameHeight, FrameHeight);
}

void CArrowButton::OnDraw(const ImVec2 & /*_Size*/)
{
  if (ImGui::ArrowButton(GetLabel().c_str(), m_Direction) && m_ClickCallback)
    m_ClickCallback();
}

//
// Setup
//

CArrowButton & CArrowButton::OnClick(std::function<void()> _Callback)
{
  m_ClickCallback = std::move(_Callback);
  return *this;
}

} // namespace editor::ui
