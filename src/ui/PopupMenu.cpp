#include "PopupMenu.h"

#include <imgui.h>

#include <optional>
#include <utility>

namespace editor::ui
{

//
// Constructor
//

CPopupMenu::CPopupMenu(const std::string & _Identifier):
  m_Label("##" + _Identifier + "PopupMenu"),
  m_IsOpenRequested(false)
{
  // Empty
}

//
// Setup
//

CPopupMenu & CPopupMenu::SetItems(std::vector<std::string> _Items)
{
  m_Items = std::move(_Items);
  return *this;
}

CPopupMenu & CPopupMenu::OnSelected(std::function<void(std::size_t)> _Callback)
{
  m_SelectCallback = std::move(_Callback);
  return *this;
}

//
// Interface
//

void CPopupMenu::Open() noexcept
{
  m_IsOpenRequested = true;
}

void CPopupMenu::Draw()
{
  if (std::exchange(m_IsOpenRequested, false))
    ImGui::OpenPopup(m_Label.c_str());

  if (!ImGui::BeginPopup(m_Label.c_str()))
    return;

  std::optional<std::size_t> Choice;

  for (std::size_t ItemIndex = 0; ItemIndex < m_Items.size(); ++ItemIndex)
  {
    ImGui::PushID(static_cast<int>(ItemIndex));

    if (ImGui::MenuItem(m_Items[ItemIndex].c_str()))
      Choice = ItemIndex;

    ImGui::PopID();
  }

  ImGui::EndPopup();

  if (Choice && m_SelectCallback)
    m_SelectCallback(*Choice);
}

} // namespace editor::ui
