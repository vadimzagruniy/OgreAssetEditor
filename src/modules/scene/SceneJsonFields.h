#pragma once

#include "SceneJson.h"

#include <OgreColourValue.h>
#include <OgreQuaternion.h>
#include <OgreVector.h>

#include <string>

namespace editor
{

bool ReadFloat(const TSceneJson & _Object, const std::string & _Key, float & _Value, std::string & _Error);

bool ReadString(const TSceneJson & _Object, const std::string & _Key, std::string & _Value, std::string & _Error);

bool ReadVector3(const TSceneJson & _Object, const std::string & _Key, Ogre::Vector3 & _Vector, std::string & _Error);

bool ReadQuaternion(const TSceneJson & _Object, const std::string & _Key, Ogre::Quaternion & _Quaternion, std::string & _Error);

bool ReadColour(const TSceneJson & _Object, const std::string & _Key, Ogre::ColourValue & _Colour, std::string & _Error);

TSceneJson WriteVector3(const Ogre::Vector3 & _Vector);

TSceneJson WriteQuaternion(const Ogre::Quaternion & _Quaternion);

TSceneJson WriteColour(const Ogre::ColourValue & _Colour);

} // namespace editor
