#pragma once

#include "AssetId.h"
#include "AssetReference.h"

#include <vector>

namespace editor
{

struct TMissingAssetReference
{
  TAssetReference       AssetRef;
  std::vector<TAssetId> ReferrerIds;
};

} // namespace editor
