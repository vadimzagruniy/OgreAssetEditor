#include "Circle.h"

#include <algorithm>

namespace editor::ui
{

//
// Constructor
//

CCircle::CCircle()
{
  // Empty
}

//
// CElement
//

void CCircle::OnDraw(const ImVec2 & _Size)
{
  const ImVec2 TopLeft = ReserveArea(_Size);
  const ImVec2 Center(TopLeft.x + _Size.x * 0.5f, TopLeft.y + _Size.y * 0.5f);
  const float  Radius = std::min(_Size.x, _Size.y) * 0.5f;

  auto & DrawList = *ImGui::GetWindowDrawList();

  if (const auto FillColor = GetFillColor())
    DrawList.AddCircleFilled(Center, Radius, *FillColor);

  if (const auto OutlineColor = GetOutlineColor())
    DrawList.AddCircle(Center, std::max(Radius - GetThickness() * 0.5f, 0.0f), *OutlineColor, 0, GetThickness());
}

} // namespace editor::ui
