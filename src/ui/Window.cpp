#include "Window.h"

#include <utility>

namespace editor::ui
{

//
// Constructor/Destructor
//

CWindow::CWindow(std::string _Title, std::string _Label):
  m_Title(std::move(_Title)),
  m_Label(std::move(_Label))
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

} // namespace editor::ui
