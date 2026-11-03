#pragma once

#include "SceneNodeProperties.h"

#include <string>
#include <vector>

namespace editor
{

struct TSceneNode
{
  std::string              Name;
  std::string              ParentName;
  std::vector<std::string> Children;
  TSceneNodeProperties     Properties;
};

} // namespace editor
