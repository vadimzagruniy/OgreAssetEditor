#include "Shell.h"

#include <imgui.h>

#include <windows.h>
#include <shellapi.h>

namespace editor::platform
{

bool RevealInExplorer(const std::filesystem::path & _Path, std::string & _Error)
{
  const std::wstring Parameters = L"/select,\"" + _Path.native() + L"\"";
  const auto         Result     = ShellExecuteW(nullptr, L"open", L"explorer.exe", Parameters.c_str(), nullptr, SW_SHOWNORMAL);

  // ShellExecute reports success with a value greater than 32.
  if (reinterpret_cast<INT_PTR>(Result) > 32)
    return true;

  _Error = "Could not open Explorer for " + _Path.string();

  return false;
}

void CopyTextToClipboard(const std::string & _Text)
{
  ImGui::SetClipboardText(_Text.c_str());
}

} // namespace editor::platform
