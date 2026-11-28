#include "AssetPath.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iterator>
#include <string>

namespace editor
{

namespace
{

constexpr const char * MetaExtension = ".meta";

} // namespace

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

std::filesystem::path MakeMetaPath(const std::filesystem::path & _AssetPath)
{
  std::filesystem::path MetaPath = _AssetPath;
  MetaPath += MetaExtension;

  return MetaPath;
}

bool IsMetaPath(const std::filesystem::path & _Path)
{
  const std::string Extension = _Path.extension().string();
  const std::string Expected  = MetaExtension;

  return std::equal(Extension.begin(), Extension.end(), Expected.begin(), Expected.end(), [](char _Left, char _Right)
  {
    return std::tolower(static_cast<unsigned char>(_Left)) == _Right;
  });
}

std::filesystem::path GetMetaAssetPath(const std::filesystem::path & _MetaPath)
{
  assert(IsMetaPath(_MetaPath));

  return _MetaPath.parent_path() / _MetaPath.stem();
}

} // namespace editor
