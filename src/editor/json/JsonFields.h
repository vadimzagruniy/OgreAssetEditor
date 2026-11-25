#pragma once

#include "Json.h"

#include <OgreColourValue.h>
#include <OgreQuaternion.h>
#include <OgreVector.h>

#include <cstdint>
#include <string>

namespace editor
{

bool ReadBool(const TJson & _Object, const std::string & _Key, bool & _Value, std::string & _Error);

bool ReadInt64(const TJson & _Object, const std::string & _Key, std::int64_t & _Value, std::string & _Error);

bool ReadFloat(const TJson & _Object, const std::string & _Key, float & _Value, std::string & _Error);

bool ReadString(const TJson & _Object, const std::string & _Key, std::string & _Value, std::string & _Error);

bool ReadVector3(const TJson & _Object, const std::string & _Key, Ogre::Vector3 & _Vector, std::string & _Error);

bool ReadQuaternion(const TJson & _Object, const std::string & _Key, Ogre::Quaternion & _Quaternion, std::string & _Error);

bool ReadColour(const TJson & _Object, const std::string & _Key, Ogre::ColourValue & _Colour, std::string & _Error);

TJson WriteVector3(const Ogre::Vector3 & _Vector);

TJson WriteQuaternion(const Ogre::Quaternion & _Quaternion);

TJson WriteColour(const Ogre::ColourValue & _Colour);

} // namespace editor
