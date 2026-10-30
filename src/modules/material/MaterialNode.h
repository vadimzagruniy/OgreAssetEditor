#pragma once

namespace editor
{

struct TMaterialNode
{
  int MaterialIndex    = -1;
  int TechniqueIndex   = -1;
  int PassIndex        = -1;
  int TextureUnitIndex = -1;

  bool operator==(const TMaterialNode &) const = default;
};

} // namespace editor
