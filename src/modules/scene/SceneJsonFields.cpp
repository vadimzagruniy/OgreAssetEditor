#include "SceneJsonFields.h"

#include <nlohmann/json.hpp>

#include <array>
#include <cstddef>

namespace editor
{

namespace
{

template <std::size_t TCount>
bool ReadNumbers(const TSceneJson & _Object, const std::string & _Key, std::array<float, TCount> & _Numbers, std::string & _Error)
{
  const auto Field = _Object.find(_Key);
  if (Field == _Object.end())
    return true;

  if (!Field->is_array() || Field->size() != TCount)
  {
    _Error = "'" + _Key + "' must be an array of " + std::to_string(TCount) + " numbers.";
    return false;
  }

  for (std::size_t NumberIndex = 0; NumberIndex < TCount; ++NumberIndex)
  {
    const auto & Element = (*Field)[NumberIndex];
    if (!Element.is_number())
    {
      _Error = "'" + _Key + "' must contain only numbers.";
      return false;
    }

    _Numbers[NumberIndex] = Element.get<float>();
  }

  return true;
}

} // namespace

bool ReadFloat(const TSceneJson & _Object, const std::string & _Key, float & _Value, std::string & _Error)
{
  const auto Field = _Object.find(_Key);
  if (Field == _Object.end())
    return true;

  if (!Field->is_number())
  {
    _Error = "'" + _Key + "' must be a number.";
    return false;
  }

  _Value = Field->get<float>();
  return true;
}

bool ReadString(const TSceneJson & _Object, const std::string & _Key, std::string & _Value, std::string & _Error)
{
  const auto Field = _Object.find(_Key);
  if (Field == _Object.end())
    return true;

  if (!Field->is_string())
  {
    _Error = "'" + _Key + "' must be a string.";
    return false;
  }

  _Value = Field->get<std::string>();
  return true;
}

bool ReadVector3(const TSceneJson & _Object, const std::string & _Key, Ogre::Vector3 & _Vector, std::string & _Error)
{
  std::array<float, 3> Numbers{_Vector.x, _Vector.y, _Vector.z};
  if (!ReadNumbers(_Object, _Key, Numbers, _Error))
    return false;

  _Vector = Ogre::Vector3(Numbers[0], Numbers[1], Numbers[2]);
  return true;
}

bool ReadQuaternion(const TSceneJson & _Object, const std::string & _Key, Ogre::Quaternion & _Quaternion, std::string & _Error)
{
  std::array<float, 4> Numbers{_Quaternion.w, _Quaternion.x, _Quaternion.y, _Quaternion.z};
  if (!ReadNumbers(_Object, _Key, Numbers, _Error))
    return false;

  _Quaternion = Ogre::Quaternion(Numbers[0], Numbers[1], Numbers[2], Numbers[3]);
  return true;
}

bool ReadColour(const TSceneJson & _Object, const std::string & _Key, Ogre::ColourValue & _Colour, std::string & _Error)
{
  std::array<float, 3> Numbers{_Colour.r, _Colour.g, _Colour.b};
  if (!ReadNumbers(_Object, _Key, Numbers, _Error))
    return false;

  _Colour = Ogre::ColourValue(Numbers[0], Numbers[1], Numbers[2]);
  return true;
}

TSceneJson WriteVector3(const Ogre::Vector3 & _Vector)
{
  return {_Vector.x, _Vector.y, _Vector.z};
}

TSceneJson WriteQuaternion(const Ogre::Quaternion & _Quaternion)
{
  return {_Quaternion.w, _Quaternion.x, _Quaternion.y, _Quaternion.z};
}

TSceneJson WriteColour(const Ogre::ColourValue & _Colour)
{
  return {_Colour.r, _Colour.g, _Colour.b};
}

} // namespace editor
