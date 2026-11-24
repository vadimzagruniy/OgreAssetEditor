#pragma once

#include "SceneNodeSnapshot.h"

#include <string>

namespace editor
{

struct TSceneNodeTemplate
{
  std::string        Identifier;
  std::string        Label;
  std::string        Category;
  std::string        BaseName;
  TSceneNodeSnapshot Snapshot;
};

} // namespace editor
