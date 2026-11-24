#pragma once

#include "SceneNodeSnapshot.h"

#include <string>

namespace editor
{

struct TSceneSubtreeNode
{
  std::string        Name;
  std::string        ParentName;
  TSceneNodeSnapshot Snapshot;
};

} // namespace editor
