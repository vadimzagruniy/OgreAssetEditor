#include "ReadFileText.h"
#include "PathUtf8.h"

#include <fstream>
#include <iterator>

namespace editor::core
{

bool ReadFileText(const std::filesystem::path & _Path, std::string & _Text, std::string & _Error)
{
  std::ifstream File(_Path, std::ios::binary);
  if (!File)
  {
    _Error = "Cannot open " + PathToUtf8(_Path.filename());
    return false;
  }

  _Text.assign(std::istreambuf_iterator<char>(File), std::istreambuf_iterator<char>());

  return true;
}

} // namespace editor::core
