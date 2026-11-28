#pragma once

#include "AssetId.h"

#include <cstdint>
#include <filesystem>

namespace editor
{

struct TAssetRecord
{
  TAssetId              Id;
  std::filesystem::path AbsolutePath;
  std::filesystem::path RelativePath;
  std::uintmax_t        SizeBytes = 0;
};

} // namespace editor
