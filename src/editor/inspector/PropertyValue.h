#pragma once

#include <OgreColourValue.h>
#include <OgreQuaternion.h>
#include <OgreVector.h>

#include <cstddef>
#include <string>
#include <variant>

namespace editor
{

using TPropertyValue = std::variant<std::monostate, bool, float, std::size_t, std::string, Ogre::Vector3, Ogre::Quaternion, Ogre::ColourValue>;

} // namespace editor
