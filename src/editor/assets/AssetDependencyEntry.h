#pragma once

#include "AssetReference.h"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace editor
{

struct TAssetDependencyEntry
{
  std::filesystem::file_time_type WriteTime;
  std::uintmax_t                  SizeBytes = 0;
  std::vector<TAssetReference>    DependencyRefs;
};

} // namespace editor
