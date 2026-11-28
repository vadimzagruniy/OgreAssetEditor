#pragma once

#include "editor/assets/AssetReference.h"
#include "modules/scene/drop/SceneDropTarget.h"

#include <string>

namespace editor
{

struct TSceneDrop
{
  TAssetReference  AssetRef;
  std::string      AssetType;
  TSceneDropTarget Target;
};

} // namespace editor
