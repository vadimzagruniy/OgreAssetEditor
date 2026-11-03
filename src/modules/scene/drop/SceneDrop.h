#pragma once

#include "modules/scene/drop/SceneDropTarget.h"

#include <filesystem>
#include <string>

namespace editor
{

struct TSceneDrop
{
  std::filesystem::path AssetPath;
  std::string           AssetType;
  TSceneDropTarget      Target;
};

} // namespace editor
