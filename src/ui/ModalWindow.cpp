#include "ModalWindow.h"

#include <imgui.h>

namespace editor::ui
{

//
// Constructor
//

CModalWindow::CModalWindow(const std::string & _Title):
  CWindow(_Title, _Title + "##Modal"),
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

//
// Interface
//

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
