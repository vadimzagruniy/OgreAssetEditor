#pragma once

#include <windows.h>
#include <shobjidl.h>

#include <filesystem>

namespace editor::platform
{

bool ReadShellItemPath(IShellItem & _Item, std::filesystem::path & _Path);

} // namespace editor::platform
