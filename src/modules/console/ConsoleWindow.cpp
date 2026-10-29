#include "ConsoleWindow.h"
#include "core/Log.h"
#include "ui/ToolBoxDesc.h"

#include <imgui.h>

#include <utility>

namespace editor
{

namespace
{

const char * GetLevelPrefix(core::ELogLevel _Level)
{
  switch (_Level)
  {
    case core::ELogLevel::Error:   return "[Error] ";
    case core::ELogLevel::Warning: return "[Warning] ";
    case core::ELogLevel::Info:    return "[Info] ";
  }

  return "";
}

} // namespace

//
// Constructor
//

CConsoleWindow::CConsoleWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<core::CLog> _Log):
  CDockedWindow(_Desc.Title, "Console", _Desc.Visible),
  m_Log(std::move(_Log))
{
  // Empty
}

//
// CWindow
//

void CConsoleWindow::OnGui()
{
  if (ImGui::Button("Clear"))
    m_Log->Clear();

  ImGui::BeginChild("##ConsoleLog", ImVec2(0, 0), ImGuiChildFlags_Borders);

  for (const auto & Entry : m_Log->GetEntries())
  {
    const std::string Line = std::string(GetLevelPrefix(Entry.Level)) + Entry.Message;
    ImGui::TextUnformatted(Line.c_str());
  }

  ImGui::EndChild();
}

} // namespace editor
