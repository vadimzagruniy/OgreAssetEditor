#include "ShellItem.h"

namespace editor::platform
{

bool ReadShellItemPath(IShellItem & _Item, std::filesystem::path & _Path)
{
  PWSTR FileSystemPath = nullptr;
  if (FAILED(_Item.GetDisplayName(SIGDN_FILESYSPATH, &FileSystemPath)))
    return false;

  _Path = FileSystemPath;
  CoTaskMemFree(FileSystemPath);

  return true;
}

} // namespace editor::platform
