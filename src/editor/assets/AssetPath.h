#pragma once

#include <filesystem>

namespace editor
{

bool IsSameOrInside(const std::filesystem::path & _Path, const std::filesystem::path & _Folder);

std::filesystem::path RebasePath(const std::filesystem::path & _Path, const std::filesystem::path & _OldFolder, const std::filesystem::path & _NewFolder);

} // namespace editor
