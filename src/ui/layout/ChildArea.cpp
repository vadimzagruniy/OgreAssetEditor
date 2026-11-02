#include "ChildArea.h"

#include <utility>

namespace editor::ui
{

//
// Constructor
//

CChildArea::CChildArea(std::string _Id, ELayoutDirection _Direction):
  CElementSetup(std::move(_Id)),
  m_Content(_Direction),
  m_WindowFlags(ImGuiWindowFlags_None),
  m_IsScrollToEnd(false)
{
  m_Content.SetWidth({ESizeMode::Auto});
}

//
// CElement
//

ImVec2 CChildArea::CalcContentSize() const
{
  return m_Content.CalcDesiredSize();
}

void CChildArea::OnDraw(const ImVec2 & _Size)
{
  if (ImGui::BeginChild(GetLabel().c_str(), _Size, ImGuiChildFlags_None, m_WindowFlags))
  {
    m_Content.Draw();

    if (m_IsScrollToEnd)
      ScrollToEnd();
  }

  ImGui::EndChild();
}

//
// Setup
//

CChildArea & CChildArea::SetWindowFlags(ImGuiWindowFlags _Flags) noexcept
{
  m_WindowFlags = _Flags;
  return *this;
}

CChildArea & CChildArea::SetScrollToEnd(bool _IsScrollToEnd) noexcept
{
  m_IsScrollToEnd = _IsScrollToEnd;
  return *this;
}

//
// Interface
//

CLayout & CChildArea::GetContent() noexcept
{
  return m_Content;
}

//
// Service
//

void CChildArea::ScrollToEnd() const
{
  if (m_Content.GetDirection() == ELayoutDirection::Row)
    ImGui::SetScrollHereX(1.0f);
  else
    ImGui::SetScrollHereY(1.0f);
}

} // namespace editor::ui
