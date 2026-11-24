#pragma once

#include "ISceneNodeBinding.h"

#include <OgrePrerequisites.h>

#include <memory>
#include <string>

namespace editor
{

struct TOgreNodeBinding
{
  Ogre::SceneNode *                  Node = nullptr;
  std::string                        TypeId;
  std::unique_ptr<ISceneNodeBinding> Object;
};

} // namespace editor
