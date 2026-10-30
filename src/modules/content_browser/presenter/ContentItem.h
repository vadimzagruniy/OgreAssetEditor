#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace editor
{

struct TContentItem
{
  std::filesystem::path RelativePath;
  std::string           Name;
  std::string           TypeName;
  std::uintmax_t        SizeBytes   = 0;
  bool                  IsDirectory = false;
};

} // namespace editor
