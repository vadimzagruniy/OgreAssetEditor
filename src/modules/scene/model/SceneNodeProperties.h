#pragma once

#include "SceneTransform.h"

#include <optional>
#include <string>

namespace editor
{

struct TSceneNodeProperties
{
  TSceneTransform            Transform;
  std::optional<std::string> MeshName;

  bool operator==(const TSceneNodeProperties &) const = default;
};

} // namespace editor
