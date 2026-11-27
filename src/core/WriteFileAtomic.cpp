#include "WriteFileAtomic.h"

#include <fstream>
#include <system_error>

namespace editor::core
{

namespace
{

std::string ToUtf8(const std::filesystem::path & _Path)
{
  const auto Utf8 = _Path.u8string();
  return std::string(Utf8.begin(), Utf8.end());
}

bool WriteWholeFile(const std::filesystem::path & _Path, std::string_view _Content)
{
  std::ofstream File(_Path, std::ios::binary | std::ios::trunc);
  File.write(_Content.data(), static_cast<std::streamsize>(_Content.size()));
  File.close();

  return static_cast<bool>(File);
}

} // namespace

bool WriteFileAtomic(const std::filesystem::path & _Path, std::string_view _Content, std::string & _Error)
{
  auto TempPath = _Path;
  TempPath += ".tmp";

  std::error_code ErrorCode;

  if (!WriteWholeFile(TempPath, _Content))
  {
    std::filesystem::remove(TempPath, ErrorCode);
    _Error = "Could not write " + ToUtf8(_Path.filename());
    return false;
  }

  std::filesystem::rename(TempPath, _Path, ErrorCode);
  if (!ErrorCode)
    return true;

  _Error = "Could not replace " + ToUtf8(_Path.filename()) + ": " + ErrorCode.message();

  std::filesystem::remove(TempPath, ErrorCode);

  return false;
}

} // namespace editor::core
