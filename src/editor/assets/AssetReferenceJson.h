#pragma once

#include "editor/json/Json.h"

#include <string>

namespace editor
{

struct TAssetReference;

bool ReadAssetReference(const TJson & _Object, const std::string & _Key, TAssetReference & _AssetRef, std::string & _Error);

TJson WriteAssetReference(const TAssetReference & _AssetRef);

} // namespace editor
