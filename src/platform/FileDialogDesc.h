#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace editor::platform
{

struct TFileDialogDesc
{
  std::string              Title;
  std::filesystem::path    RootFolder;
  std::filesystem::path    InitialFolder;
  std::string              FilterName;
  std::vector<std::string> Extensions;
};

} // namespace editor::platform
