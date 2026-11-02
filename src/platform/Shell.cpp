#include "Shell.h"

#include <imgui.h>

#include <windows.h>
#include <shellapi.h>

namespace editor::platform
{

namespace
{

bool RunShell(const wchar_t * _File, const wchar_t * _Parameters)
{
  const auto Result = ShellExecuteW(nullptr, L"open", _File, _Parameters, nullptr, SW_SHOWNORMAL);

  // ShellExecute reports success with a value greater than 32.
  return reinterpret_cast<INT_PTR>(Result) > 32;
}

} // namespace

bool RevealInExplorer(const std::filesystem::path & _Path, std::string & _Error)
{
  const std::wstring Parameters = L"/select,\"" + _Path.native() + L"\"";

  if (RunShell(L"explorer.exe", Parameters.c_str()))
    return true;

  _Error = "Could not open Explorer for " + _Path.string();

  return false;
}

bool OpenWithDefaultApp(const std::filesystem::path & _Path, std::string & _Error)
{
  if (RunShell(_Path.c_str(), nullptr))
    return true;

  _Error = "No application is associated with " + _Path.string();

  return false;
}

void CopyTextToClipboard(const std::string & _Text)
{
  ImGui::SetClipboardText(_Text.c_str());
}

} // namespace editor::platform
