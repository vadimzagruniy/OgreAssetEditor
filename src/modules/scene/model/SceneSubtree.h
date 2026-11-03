#pragma once

#include "SceneSubtreeNode.h"

#include <cstddef>
#include <vector>

namespace editor
{

struct TSceneSubtree
{
  std::size_t                    Index = 0;
  std::vector<TSceneSubtreeNode> Nodes;
};

} // namespace editor
