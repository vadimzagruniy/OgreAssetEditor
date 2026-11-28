#pragma once

#include <filesystem>

namespace editor
{

bool IsSameOrInside(const std::filesystem::path & _Path, const std::filesystem::path & _Folder);

std::filesystem::path RebasePath(const std::filesystem::path & _Path, const std::filesystem::path & _OldFolder, const std::filesystem::path & _NewFolder);

std::filesystem::path MakeMetaPath(const std::filesystem::path & _AssetPath);

bool IsMetaPath(const std::filesystem::path & _Path);

std::filesystem::path GetMetaAssetPath(const std::filesystem::path & _MetaPath);

} // namespace editor
