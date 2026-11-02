#include "ComboBox.h"

#include <algorithm>
#include <utility>

namespace editor::ui
{

//
// Constructor
//

CComboBox::CComboBox(std::string _Label):
  CElementSetup(std::move(_Label))
{
  // Empty
}

//
// CElement
//

ImVec2 CComboBox::CalcContentSize() const
{
  return CalcDefaultItemSize();
}

void CComboBox::OnDraw(const ImVec2 & _Size)
{
  ImGui::SetNextItemWidth(std::max(_Size.x - CalcSideLabelWidth(), 1.0f));

  if (!ImGui::BeginCombo(GetLabel().c_str(), GetPreview().c_str()))
    return;

  const auto Choice = DrawItems();

  ImGui::EndCombo();

  if (!Choice)
    return;

  m_SelectedIndex = Choice;

  if (m_SelectCallback)
    m_SelectCallback(*Choice);
}

//
// Setup
//

CComboBox & CComboBox::SetItems(std::vector<std::string> _Items)
{
  m_Items = std::move(_Items);
  return *this;
}

CComboBox & CComboBox::SetSelectedIndex(std::optional<std::size_t> _SelectedIndex) noexcept
{
  m_SelectedIndex = _SelectedIndex;
  return *this;
}

CComboBox & CComboBox::SetPlaceholder(std::string _Placeholder)
{
  m_Placeholder = std::move(_Placeholder);
  return *this;
}

CComboBox & CComboBox::OnSelected(std::function<void(std::size_t)> _Callback)
{
  m_SelectCallback = std::move(_Callback);
  return *this;
}

//
// Interface
//

const std::vector<std::string> & CComboBox::GetItems() const noexcept
{
  return m_Items;
}

std::optional<std::size_t> CComboBox::GetSelectedIndex() const noexcept
{
  return m_SelectedIndex;
}

//
// Service
//

const std::string & CComboBox::GetPreview() const noexcept
{
  if (m_SelectedIndex && *m_SelectedIndex < m_Items.size())
    return m_Items[*m_SelectedIndex];

  return m_Placeholder;
}

std::optional<std::size_t> CComboBox::DrawItems() const
{
  std::optional<std::size_t> Choice;

  for (std::size_t ItemIndex = 0; ItemIndex < m_Items.size(); ++ItemIndex)
  {
    const bool IsSelected = m_SelectedIndex == ItemIndex;

    ImGui::PushID(static_cast<int>(ItemIndex));

    if (ImGui::Selectable(m_Items[ItemIndex].c_str(), IsSelected) && !IsSelected)
      Choice = ItemIndex;

    if (IsSelected)
      ImGui::SetItemDefaultFocus();

    ImGui::PopID();
  }

  return Choice;
}

} // namespace editor::ui
