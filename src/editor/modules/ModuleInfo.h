#pragma once

#include <string>
#include <vector>

namespace editor
{

struct TModuleInfo
{
  std::string              Name;
  std::vector<std::string> Dependencies;
};

} // namespace editor
