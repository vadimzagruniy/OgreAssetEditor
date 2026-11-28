#pragma once

#include "editor/assets/AssetReference.h"

#include <map>
#include <string>

namespace editor
{

struct TSceneAnimation
{
  TAssetReference                    AssetRef;
  std::map<std::string, std::string> Bindings;

  bool operator==(const TSceneAnimation &) const = default;
};

} // namespace editor
