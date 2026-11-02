#pragma once

#include <OgreVector.h>

namespace editor
{

struct TCameraPose
{
  Ogre::Vector3 Pivot    = Ogre::Vector3::ZERO;
  float         Yaw      = 0.0f;
  float         Pitch    = 0.0f;
  float         Distance = 1.0f;
};

} // namespace editor
