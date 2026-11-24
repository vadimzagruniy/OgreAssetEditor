#include "ModalWindow.h"
#include "ui/elements/Button.h"

#include <imgui.h>

#include <utility>

namespace editor::ui
{

namespace
{

constexpr float ContentWidthScale = 30.0f;
constexpr float ButtonWidthScale  = 7.0f;

} // namespace

//
// Constructor
//

CModalWindow::CModalWindow(const std::string & _Title):
  CWindow(_Title, _Title + "##Modal"),
  m_Buttons(ELayoutDirection::Row, ELayoutAlign::Center),
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

  if (!ImGui::BeginPopupModal(GetLabel().c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
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
  const float ContentWidth = ImGui::GetFontSize() * ContentWidthScale;

  if (ImGui::BeginChild("##Content", ImVec2(ContentWidth, 0.0f), ImGuiChildFlags_AutoResizeY))
    OnContentGui();

  ImGui::EndChild();

  ImGui::Separator();
  ImGui::Spacing();

  m_Buttons.Draw();
}

//
// Interface
//

CButton & CModalWindow::AddButton(std::string _Label, std::function<void()> _Action)
{
  return m_Buttons.Add(CButton(std::move(_Label))).SetWidth(ImGui::GetFontSize() * ButtonWidthScale).OnClick(std::move(_Action));
}

void CModalWindow::Close() noexcept
{
  m_IsClosed = true;
}

//
// Implementation
//

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
