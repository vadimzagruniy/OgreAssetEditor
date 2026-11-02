#include "Rectangle.h"

namespace editor::ui
{

//
// Constructor
//

CRectangle::CRectangle():
  m_Rounding(0.0f)
{
  // Empty
}

//
// CElement
//

void CRectangle::OnDraw(const ImVec2 & _Size)
{
  const ImVec2 TopLeft = ReserveArea(_Size);
  const ImVec2 BottomRight(TopLeft.x + _Size.x, TopLeft.y + _Size.y);

  auto & DrawList = *ImGui::GetWindowDrawList();

  if (const auto FillColor = GetFillColor())
    DrawList.AddRectFilled(TopLeft, BottomRight, *FillColor, m_Rounding);

  if (const auto OutlineColor = GetOutlineColor())
    DrawList.AddRect(TopLeft, BottomRight, *OutlineColor, m_Rounding, ImDrawFlags_None, GetThickness());
}

//
// Setup
//

CRectangle & CRectangle::SetRounding(float _Rounding) noexcept
{
  m_Rounding = _Rounding;
  return *this;
}

} // namespace editor::ui
