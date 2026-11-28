#pragma once

#include "editor/json/Json.h"

#include <string>
#include <vector>

namespace editor
{

struct TAssetReference;

bool ReadAssetReference(const TJson & _Object, const std::string & _Key, TAssetReference & _AssetRef, std::string & _Error);

TJson WriteAssetReference(const TAssetReference & _AssetRef);

void CollectAssetReferences(const TJson & _Json, std::vector<TAssetReference> & _AssetRefs);

} // namespace editor
