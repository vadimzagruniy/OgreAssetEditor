#pragma once

#include <filesystem>
#include <string>

namespace editor::core
{

std::string PathToUtf8(const std::filesystem::path & _Path);

} // namespace editor::core
