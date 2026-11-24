#pragma once

#include "SceneTransform.h"
#include "modules/scene/SceneJson.h"

#include <nlohmann/json.hpp>

#include <string>

namespace editor
{

struct TSceneNodeSnapshot
{
  std::string     TypeId;
  TSceneTransform Transform;
  TSceneJson      Data = TSceneJson::object();

  bool operator==(const TSceneNodeSnapshot &) const = default;
};

} // namespace editor
