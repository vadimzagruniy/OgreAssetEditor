#include "Text.h"

#include <utility>

namespace editor::ui
{

//
// Constructor
//

CText::CText(std::string _Text):
  CElementSetup(std::move(_Text))
{
  // Empty
}

//
// CElement
//

ImVec2 CText::CalcContentSize() const
{
  return ImGui::CalcTextSize(GetLabel().c_str());
}

void CText::OnDraw(const ImVec2 & /*_Size*/)
{
  ImGui::TextUnformatted(GetLabel().c_str());
}

} // namespace editor::ui
