#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace editor::core
{

bool WriteFileAtomic(const std::filesystem::path & _Path, std::string_view _Content, std::string & _Error);

} // namespace editor::core
