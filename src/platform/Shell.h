#pragma once

#include <filesystem>
#include <string>

namespace editor::platform
{

bool RevealInExplorer(const std::filesystem::path & _Path, std::string & _Error);

bool OpenWithDefaultApp(const std::filesystem::path & _Path, std::string & _Error);

void CopyTextToClipboard(const std::string & _Text);

} // namespace editor::platform
