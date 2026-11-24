#include "GroupPropertyControl.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor
//

CGroupPropertyControl::CGroupPropertyControl(std::string _Label, std::vector<std::unique_ptr<IPropertyControl>> _Children):
  m_Label(std::move(_Label)),
  m_Children(std::move(_Children))
{
  // Empty
}

//
// IPropertyControl
//

void CGroupPropertyControl::Draw(IPropertySource & _Source)
{
  if (!ImGui::CollapsingHeader(m_Label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    return;

  ImGui::PushID(m_Label.c_str());

  for (const auto & Child : m_Children)
    Child->Draw(_Source);

  ImGui::PopID();
}

} // namespace editor
