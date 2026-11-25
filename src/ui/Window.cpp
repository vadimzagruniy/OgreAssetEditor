#include "Window.h"

#include <imgui.h>

#include <utility>

namespace editor::ui
{

//
// Constructor/Destructor
//

CWindow::CWindow(std::string _Title, std::string _Label):
  m_Title(std::move(_Title)),
  m_Label(std::move(_Label)),
  m_FocusFrame(-1)
{
  // Empty
}

CWindow::~CWindow() = default;

//
// Interface
//

void CWindow::Draw()
{
  if (!IsOpen())
    return;

  const bool IsBegun = BeginWindow();

  if (IsBegun && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    m_FocusFrame = ImGui::GetFrameCount();

  if (IsBegun && IsOpen())
    OnGui();

  EndWindow(IsBegun);
}

const std::string & CWindow::GetTitle() const noexcept
{
  return m_Title;
}

const std::string & CWindow::GetLabel() const noexcept
{
  return m_Label;
}

int CWindow::GetFocusFrame() const noexcept
{
  return m_FocusFrame;
}

} // namespace editor::ui
