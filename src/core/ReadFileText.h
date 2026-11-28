#pragma once

#include <filesystem>
#include <string>

namespace editor::core
{

bool ReadFileText(const std::filesystem::path & _Path, std::string & _Text, std::string & _Error);

} // namespace editor::core
