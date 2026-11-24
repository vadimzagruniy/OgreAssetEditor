#pragma once

#include "SceneChangeKind.h"

#include <string>

namespace editor
{

struct TSceneChange
{
  ESceneChangeKind Kind = ESceneChangeKind::Changed;
  std::string      Name;
  std::string      NewName;
};

} // namespace editor
