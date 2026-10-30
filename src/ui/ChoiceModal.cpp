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
  // Empty
}

//
// CModalWindow
//

void CChoiceModal::OnGui()
{
  ImGui::TextUnformatted(m_Desc.Message.c_str());

  for (std::size_t DetailIndex = 0; DetailIndex < m_Desc.Details.size(); ++DetailIndex)
  {
    if (DetailIndex == MaxListedDetails)
    {
      ImGui::TextDisabled("...");
      break;
    }

    ImGui::BulletText("%s", m_Desc.Details[DetailIndex].c_str());
  }

  if (!m_Error.empty())
    ImGui::TextColored(ErrorColour, "%s", m_Error.c_str());

  ImGui::Separator();

  for (std::size_t ChoiceIndex = 0; ChoiceIndex < m_Desc.Choices.size(); ++ChoiceIndex)
  {
    if (ChoiceIndex > 0)
      ImGui::SameLine();

    const auto & Choice = m_Desc.Choices[ChoiceIndex];
    if (!ImGui::Button(Choice.Label.c_str()))
      continue;

    m_Error.clear();

    if (!Choice.Action || Choice.Action(m_Error))
    {
      Close();
      break;
    }
  }
}

void CChoiceModal::OnCancel()
{
  if (m_Desc.CancelAction)
    m_Desc.CancelAction();
}

} // namespace editor::ui
