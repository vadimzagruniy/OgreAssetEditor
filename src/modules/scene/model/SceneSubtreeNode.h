#pragma once

#include "SceneNodeProperties.h"

#include <string>

namespace editor
{

struct TSceneSubtreeNode
{
  std::string          Name;
  std::string          ParentName;
  TSceneNodeProperties Properties;
};

} // namespace editor
