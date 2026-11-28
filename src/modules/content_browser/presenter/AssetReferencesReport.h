#pragma once

#include <string>
#include <vector>

namespace editor
{

struct TAssetReferencesReport
{
  std::string              AssetPath;
  std::vector<std::string> ReferrerPaths;
};

} // namespace editor
