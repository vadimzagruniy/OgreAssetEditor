#pragma once

#include "SceneTransform.h"
#include "editor/json/Json.h"

#include <nlohmann/json.hpp>

#include <string>

namespace editor
{

struct TSceneNodeSnapshot
{
  std::string     TypeId;
  TSceneTransform Transform;
  TJson           Data = TJson::object();

  bool operator==(const TSceneNodeSnapshot &) const = default;
};

} // namespace editor
