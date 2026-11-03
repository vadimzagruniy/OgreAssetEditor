#pragma once

#include <OgrePrerequisites.h>

#include <optional>
#include <string>

namespace editor
{

struct TOgreNodeBinding
{
  Ogre::SceneNode *          Node   = nullptr;
  Ogre::Entity *             Entity = nullptr;
  std::optional<std::string> MeshName;
};

} // namespace editor
