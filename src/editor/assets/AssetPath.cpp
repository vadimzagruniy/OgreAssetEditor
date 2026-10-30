#include "AssetPath.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace editor
{

bool IsSameOrInside(const std::filesystem::path & _Path, const std::filesystem::path & _Folder)
{
  const auto FolderDistance = std::distance(_Folder.begin(), _Folder.end());
  const auto PathDistance   = std::distance(_Path.begin(), _Path.end());

  if (FolderDistance > PathDistance)
    return false;

  return std::equal(_Folder.begin(), _Folder.end(), _Path.begin());
}

std::filesystem::path RebasePath(const std::filesystem::path & _Path, const std::filesystem::path & _OldFolder, const std::filesystem::path & _NewFolder)
{
  assert(IsSameOrInside(_Path, _OldFolder));

  auto PathElement = _Path.begin();
  std::advance(PathElement, std::distance(_OldFolder.begin(), _OldFolder.end()));

  std::filesystem::path Result = _NewFolder;

  for (; PathElement != _Path.end(); ++PathElement)
    Result /= *PathElement;

  return Result;
}

} // namespace editor
