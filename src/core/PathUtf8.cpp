#include "PathUtf8.h"

namespace editor::core
{

std::string PathToUtf8(const std::filesystem::path & _Path)
{
  const auto Utf8 = _Path.u8string();
  return std::string(Utf8.begin(), Utf8.end());
}

} // namespace editor::core
