#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace editor::platform
{

struct TFileDialogDesc;

bool OpenFileDialog(const TFileDialogDesc & _Desc, std::filesystem::path & _Selected, std::string & _Error);

std::optional<std::filesystem::path> FindRelativePath(const std::filesystem::path & _Path, const std::filesystem::path & _Folder);

} // namespace editor::platform
