#include "Window.h"

#include <imgui.h>

#include <utility>

namespace editor::ui
{

//
// Constructor/Destructor
//

CWindow::CWindow(std::string _Title, std::string _StableId, bool _Visible):
  m_Title(std::move(_Title)),
  m_Label(m_Title + "###" + _StableId),
  m_Visible(_Visible)
{
  // Empty
}

CWindow::~CWindow() = default;

//
// Interface
//

void CWindow::Draw()
{
  if (!m_Visible)
    return;

  if (ImGui::Begin(m_Label.c_str(), &m_Visible))
    OnGui();

  ImGui::End();
}

bool CWindow::IsVisible() const noexcept
{
  return m_Visible;
}

void CWindow::SetVisible(bool _Visible) noexcept
{
  m_Visible = _Visible;
}

const std::string & CWindow::GetTitle() const noexcept
{
  return m_Title;
}

const std::string & CWindow::GetLabel() const noexcept
{
  return m_Label;
}

} // namespace editor::ui
