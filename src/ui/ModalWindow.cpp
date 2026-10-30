#include "ModalWindow.h"

#include <imgui.h>

#include <utility>

namespace editor::ui
{

//
// Constructor/Destructor
//

CModalWindow::CModalWindow(std::string _Title):
  m_Title(std::move(_Title)),
  m_Label(m_Title + "##Modal"),
  m_IsOpened(false),
  m_IsClosed(false)
{
  // Empty
}

CModalWindow::~CModalWindow() = default;

//
// Interface
//

bool CModalWindow::Draw()
{
  if (m_IsClosed)
    return false;

  if (!m_IsOpened)
  {
    ImGui::OpenPopup(m_Label.c_str());
    m_IsOpened = true;
  }

  if (!ImGui::BeginPopupModal(m_Label.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
  {
    OnCancel();
    m_IsClosed = true;
    return false;
  }

  if (ImGui::Shortcut(ImGuiKey_Escape))
  {
    OnCancel();
    Close();
  }
  else
  {
    OnGui();
  }

  if (m_IsClosed)
    ImGui::CloseCurrentPopup();

  ImGui::EndPopup();

  return !m_IsClosed;
}

const std::string & CModalWindow::GetTitle() const noexcept
{
  return m_Title;
}

void CModalWindow::Close() noexcept
{
  m_IsClosed = true;
}

//
// Events
//

void CModalWindow::OnCancel()
{
  // Empty
}

} // namespace editor::ui
