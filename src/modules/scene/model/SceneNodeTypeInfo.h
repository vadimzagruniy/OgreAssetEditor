#pragma once

#include "SceneNodeIcon.h"

#include <string>

namespace editor
{

struct TSceneNodeTypeInfo
{
  std::string    TypeId;
  std::string    Label;
  std::string    Category;
  ESceneNodeIcon Icon = ESceneNodeIcon::None;
};

} // namespace editor
