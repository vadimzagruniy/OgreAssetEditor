#include "Spacer.h"

#include <string>

namespace editor::ui
{

//
// Constructor
//

CSpacer::CSpacer():
  CElementSetup(std::string())
{
  SetWidth({ESizeMode::Fill});
  SetHeight({ESizeMode::Fill});
}

//
// CElement
//

ImVec2 CSpacer::CalcContentSize() const
{
  return ImVec2(0.0f, 0.0f);
}

void CSpacer::OnDraw(const ImVec2 & _Size)
{
  ImGui::Dummy(_Size);
}

} // namespace editor::ui
