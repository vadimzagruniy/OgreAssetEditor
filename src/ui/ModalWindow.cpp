#include "ModalWindow.h"

#include <imgui.h>

#include <algorithm>
#include <optional>
#include <utility>

namespace editor::ui
{

namespace
{

constexpr float ButtonWidthScale = 10.0f;

float CalcButtonsPanelHeight()
{
  const ImGuiStyle & Style = ImGui::GetStyle();
  return ImGui::GetFrameHeight() + Style.ItemSpacing.y * 2.0f + Style.WindowPadding.y;
}

} // namespace

//
// Constructor
//

CModalWindow::CModalWindow(const std::string & _Title, const ImVec2 & _Size):
  CWindow(_Title, _Title + "##Modal"),
  m_Size(_Size),
  m_IsOpened(false),
  m_IsClosed(false)
{
  // Empty
}

//
// CWindow
//

bool CModalWindow::IsOpen() const noexcept
{
  return !m_IsClosed;
}

bool CModalWindow::BeginWindow()
{
  if (!m_IsOpened)
  {
    ImGui::OpenPopup(GetLabel().c_str());
    m_IsOpened = true;
  }

  const float FontSize = ImGui::GetFontSize();
  ImGui::SetNextWindowSize(ImVec2(FontSize * m_Size.x, FontSize * m_Size.y), ImGuiCond_Appearing);

  bool IsVisible = true;
  if (!ImGui::BeginPopupModal(GetLabel().c_str(), &IsVisible, ImGuiWindowFlags_NoCollapse))
  {
    Cancel();
    return false;
  }

  if (ImGui::Shortcut(ImGuiKey_Escape))
    Cancel();

  return true;
}

void CModalWindow::EndWindow(bool _IsBegun)
{
  if (!_IsBegun)
    return;

  if (m_IsClosed)
    ImGui::CloseCurrentPopup();

  ImGui::EndPopup();
}

void CModalWindow::OnGui()
{
  if (ImGui::BeginChild("##Content", ImVec2(0.0f, -CalcButtonsPanelHeight())))
    OnContentGui();

  ImGui::EndChild();

  ImGui::Separator();
  ImGui::Spacing();

  DrawButtons();
}

//
// Interface
//

std::size_t CModalWindow::AddButton(std::string _Label, std::function<void()> _Action)
{
  m_Buttons.push_back(TModalButton{std::move(_Label), std::move(_Action)});
  return m_Buttons.size() - 1;
}

void CModalWindow::SetButtonEnabled(std::size_t _ButtonIndex, bool _IsEnabled)
{
  m_Buttons.at(_ButtonIndex).IsEnabled = _IsEnabled;
}

void CModalWindow::Close() noexcept
{
  m_IsClosed = true;
}

//
// Implementation
//

void CModalWindow::DrawButtons()
{
  if (m_Buttons.empty())
    return;

  const float  Spacing      = ImGui::GetStyle().ItemSpacing.x;
  const float  ButtonCount  = static_cast<float>(m_Buttons.size());
  const ImVec2 ButtonSize(ImGui::GetFontSize() * ButtonWidthScale, ImGui::GetFrameHeight());
  const float  ButtonsWidth = ButtonSize.x * ButtonCount + Spacing * (ButtonCount - 1.0f);

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max((ImGui::GetContentRegionAvail().x - ButtonsWidth) * 0.5f, 0.0f));

  std::optional<std::size_t> ClickedIndex;

  for (std::size_t ButtonIndex = 0; ButtonIndex < m_Buttons.size(); ++ButtonIndex)
  {
    const auto & Button = m_Buttons[ButtonIndex];

    if (ButtonIndex > 0)
      ImGui::SameLine();

    ImGui::BeginDisabled(!Button.IsEnabled);

    if (ImGui::Button(Button.Label.c_str(), ButtonSize))
      ClickedIndex = ButtonIndex;

    ImGui::EndDisabled();
  }

  if (ClickedIndex && m_Buttons[*ClickedIndex].Action)
    m_Buttons[*ClickedIndex].Action();
}

void CModalWindow::Cancel()
{
  OnCancel();
  Close();
}

//
// Events
//

void CModalWindow::OnCancel()
{
  // Empty
}

} // namespace editor::ui
