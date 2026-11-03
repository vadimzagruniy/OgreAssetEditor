#pragma once

#include <OgreQuaternion.h>
#include <OgreVector.h>

namespace editor
{

struct TSceneTransform
{
  Ogre::Vector3    Position    = Ogre::Vector3::ZERO;
  Ogre::Quaternion Orientation = Ogre::Quaternion::IDENTITY;
  Ogre::Vector3    Scale       = Ogre::Vector3::UNIT_SCALE;

  bool operator==(const TSceneTransform &) const = default;
};

} // namespace editor
