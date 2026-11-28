#pragma once

#include "AssetId.h"

#include <string>

namespace editor
{

struct TAssetReference
{
  TAssetId    Id;
  std::string PathHint;

  bool IsEmpty() const noexcept;

  bool IsSameAsset(const TAssetReference & _Other) const noexcept;

  bool operator==(const TAssetReference & _Other) const noexcept;

  bool operator!=(const TAssetReference & _Other) const noexcept;
};

} // namespace editor
