#include "ChoiceModal.h"

#include <imgui.h>

#include <cstddef>
#include <utility>

namespace editor::ui
{

namespace
{

constexpr std::size_t MaxListedDetails = 8;

const ImVec4 ErrorColour(1.0f, 0.4f, 0.4f, 1.0f);

} // namespace

//
// Constructor
//

CChoiceModal::CChoiceModal(TChoiceModalDesc _Desc):
  CModalWindow(_Desc.Title),
  m_Desc(std::move(_Desc))
{
  for (std::size_t ChoiceIndex = 0; ChoiceIndex < m_Desc.Choices.size(); ++ChoiceIndex)
  {
    AddButton(m_Desc.Choices[ChoiceIndex].Label, [this, ChoiceIndex]
    {
      OnChoiceClicked(ChoiceIndex);
    });
  }
}

//
// CModalWindow
//

void CChoiceModal::OnContentGui()
{
  ImGui::TextWrapped("%s", m_Desc.Message.c_str());

  for (std::size_t DetailIndex = 0; DetailIndex < m_Desc.Details.size(); ++DetailIndex)
  {
    if (DetailIndex == MaxListedDetails)
    {
      ImGui::TextDisabled("...");
      break;
    }

    ImGui::BulletText("%s", m_Desc.Details[DetailIndex].c_str());
  }

  if (m_Error.empty())
    return;

  ImGui::PushStyleColor(ImGuiCol_Text, ErrorColour);
  ImGui::TextWrapped("%s", m_Error.c_str());
  ImGui::PopStyleColor();
}

void CChoiceModal::OnCancel()
{
  if (m_Desc.CancelAction)
    m_Desc.CancelAction();
}

//
// Events
//

void CChoiceModal::OnChoiceClicked(std::size_t _ChoiceIndex)
{
  const auto & Choice = m_Desc.Choices[_ChoiceIndex];

  m_Error.clear();

  if (!Choice.Action || Choice.Action(m_Error))
    Close();
}

} // namespace editor::ui
