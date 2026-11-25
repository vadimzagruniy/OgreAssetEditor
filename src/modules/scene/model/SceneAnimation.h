#pragma once

#include <map>
#include <string>

namespace editor
{

struct TSceneAnimation
{
  std::string                        Path;
  std::map<std::string, std::string> Bindings;

  bool operator==(const TSceneAnimation &) const = default;
};

} // namespace editor
