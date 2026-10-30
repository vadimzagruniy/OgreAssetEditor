#pragma once

#include "AssetRecord.h"

#include <string>
#include <vector>

namespace editor
{

struct TAssetNode
{
  TAssetRecord            Record;
  std::string             Name;
  bool                    IsDirectory = false;
  std::vector<TAssetNode> Children;
};

} // namespace editor
