#include "AssetReference.h"

namespace editor
{

bool TAssetReference::IsEmpty() const noexcept
{
  return !Id.IsValid() && PathHint.empty();
}

bool TAssetReference::IsSameAsset(const TAssetReference & _Other) const noexcept
{
  if (Id.IsValid() || _Other.Id.IsValid())
    return Id == _Other.Id;

  return PathHint == _Other.PathHint;
}

bool TAssetReference::operator==(const TAssetReference & _Other) const noexcept
{
  return Id == _Other.Id && PathHint == _Other.PathHint;
}

bool TAssetReference::operator!=(const TAssetReference & _Other) const noexcept
{
  return !(*this == _Other);
}

} // namespace editor
