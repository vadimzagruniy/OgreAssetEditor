#include "DockedWindow.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace editor::ui
{

//
// Constructor
//

CDockedWindow::CDockedWindow(std::string _Title, const std::string & _StableId, bool _Visible):
  CWindow(_Title, _Title + "###" + _StableId),
  m_Visible(_Visible),
  m_IsTitleBarHidden(false)
{
  // Empty
}

//
// CWindow
//

bool CDockedWindow::IsOpen() const noexcept
{
  return m_Visible;
}

bool CDockedWindow::BeginWindow()
{
  if (m_IsTitleBarHidden)
  {
    ImGuiWindowClass WindowClass;
    WindowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

    ImGui::SetNextWindowClass(&WindowClass);
  }

  return ImGui::Begin(GetLabel().c_str(), &m_Visible);
}

void CDockedWindow::EndWindow(bool /*_IsBegun*/)
{
  ImGui::End();
}

//
// Interface
//

bool CDockedWindow::IsVisible() const noexcept
{
  return m_Visible;
}

void CDockedWindow::SetVisible(bool _Visible) noexcept
{
  m_Visible = _Visible;
}

void CDockedWindow::SetTitleBarHidden(bool _IsTitleBarHidden) noexcept
{
  m_IsTitleBarHidden = _IsTitleBarHidden;
}

} // namespace editor::ui
