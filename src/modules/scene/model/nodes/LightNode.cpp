#include "LightNode.h"
#include "editor/inspector/PropertyDesc.h"
#include "editor/json/JsonFields.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <utility>

namespace editor
{

namespace
{

struct TLightTypeOption
{
  ELightType   LightType;
  const char * Key;
  const char * Label;
};

constexpr std::array<TLightTypeOption, 3> LightTypeOptions{{
  {ELightType::Point,       "point",       "Point"      },
  {ELightType::Directional, "directional", "Directional"},
  {ELightType::Spot,        "spot",        "Spot"       }
}};

const std::string TypeIdentifier      = "Light.Type";
const std::string ColourIdentifier    = "Light.Colour";
const std::string IntensityIdentifier = "Light.Intensity";
const std::string RangeIdentifier     = "Light.Range";
const std::string InnerIdentifier     = "Light.SpotInner";
const std::string OuterIdentifier     = "Light.SpotOuter";

const std::string TypeKey      = "light";
const std::string ColourKey    = "colour";
const std::string IntensityKey = "intensity";
const std::string RangeKey     = "range";
const std::string InnerKey     = "spotInner";
const std::string OuterKey     = "spotOuter";

std::size_t FindLightTypeIndex(ELightType _LightType)
{
  const auto Option = std::find_if(LightTypeOptions.begin(), LightTypeOptions.end(), [_LightType](const TLightTypeOption & _Option)
  {
    return _Option.LightType == _LightType;
  });

  return static_cast<std::size_t>(std::distance(LightTypeOptions.begin(), Option));
}

template <class TField>
bool AssignValue(TField & _Field, const TPropertyValue & _Value)
{
  const auto * Value = std::get_if<TField>(&_Value);
  if (!Value)
    return false;

  _Field = *Value;
  return true;
}

} // namespace

//
// Constructor
//

CLightNode::CLightNode():
  m_LightType(ELightType::Point),
  m_Colour(Ogre::ColourValue::White),
  m_Intensity(1.0f),
  m_Range(1000.0f),
  m_SpotInnerAngle(30.0f),
  m_SpotOuterAngle(45.0f)
{
  // Empty
}

//
// CSceneNode
//

std::string_view CLightNode::GetTypeId() const noexcept
{
  return TypeId;
}

void CLightNode::DescribeProperties(std::vector<TPropertyDesc> & _Properties) const
{
  CSceneNode::DescribeProperties(_Properties);

  std::vector<std::string> TypeLabels;
  for (const auto & Option : LightTypeOptions)
    TypeLabels.emplace_back(Option.Label);

  TPropertyDesc Light{"Light", "Light", EPropertyType::Group};
  Light.Children.push_back({TypeIdentifier,      "Type",       EPropertyType::Enum,  {.Options = std::move(TypeLabels)}});
  Light.Children.push_back({ColourIdentifier,    "Colour",     EPropertyType::Color, {}});
  Light.Children.push_back({IntensityIdentifier, "Intensity",  EPropertyType::Float, {.Minimum = 0.0f, .Maximum = 100.0f,    .Speed = 0.05f, .Format = "%.2f"}});
  Light.Children.push_back({RangeIdentifier,     "Range",      EPropertyType::Float, {.Minimum = 0.0f, .Maximum = 100000.0f, .Speed = 1.0f,  .Format = "%.1f"}});
  Light.Children.push_back({InnerIdentifier,     "Spot Inner", EPropertyType::Float, {.Minimum = 0.0f, .Maximum = 179.0f,    .Speed = 0.5f,  .Format = "%.1f"}});
  Light.Children.push_back({OuterIdentifier,     "Spot Outer", EPropertyType::Float, {.Minimum = 0.0f, .Maximum = 179.0f,    .Speed = 0.5f,  .Format = "%.1f"}});

  _Properties.push_back(std::move(Light));
}

TPropertyValue CLightNode::GetValue(const std::string & _Identifier) const
{
  if (_Identifier == TypeIdentifier)
    return FindLightTypeIndex(m_LightType);

  if (_Identifier == ColourIdentifier)
    return m_Colour;

  if (_Identifier == IntensityIdentifier)
    return m_Intensity;

  if (_Identifier == RangeIdentifier)
    return m_Range;

  if (_Identifier == InnerIdentifier)
    return m_SpotInnerAngle;

  if (_Identifier == OuterIdentifier)
    return m_SpotOuterAngle;

  return CSceneNode::GetValue(_Identifier);
}

bool CLightNode::SetValue(const std::string & _Identifier, const TPropertyValue & _Value)
{
  if (_Identifier == TypeIdentifier)
  {
    const auto * OptionIndex = std::get_if<std::size_t>(&_Value);
    if (!OptionIndex || *OptionIndex >= LightTypeOptions.size())
      return false;

    m_LightType = LightTypeOptions[*OptionIndex].LightType;
    return true;
  }

  if (_Identifier == ColourIdentifier)
    return AssignValue(m_Colour, _Value);

  if (_Identifier == IntensityIdentifier)
    return AssignValue(m_Intensity, _Value);

  if (_Identifier == RangeIdentifier)
    return AssignValue(m_Range, _Value);

  if (_Identifier == InnerIdentifier)
    return AssignValue(m_SpotInnerAngle, _Value);

  if (_Identifier == OuterIdentifier)
    return AssignValue(m_SpotOuterAngle, _Value);

  return CSceneNode::SetValue(_Identifier, _Value);
}

void CLightNode::OnWriteData(TJson & _Data) const
{
  _Data[TypeKey]      = LightTypeOptions[FindLightTypeIndex(m_LightType)].Key;
  _Data[ColourKey]    = WriteColour(m_Colour);
  _Data[IntensityKey] = m_Intensity;
  _Data[RangeKey]     = m_Range;
  _Data[InnerKey]     = m_SpotInnerAngle;
  _Data[OuterKey]     = m_SpotOuterAngle;
}

bool CLightNode::OnReadData(const TJson & _Data, std::string & _Error)
{
  std::string TypeName = LightTypeOptions[FindLightTypeIndex(m_LightType)].Key;

  if (!ReadString(_Data, TypeKey, TypeName, _Error) ||
      !ReadColour(_Data, ColourKey, m_Colour, _Error) ||
      !ReadFloat(_Data, IntensityKey, m_Intensity, _Error) ||
      !ReadFloat(_Data, RangeKey, m_Range, _Error) ||
      !ReadFloat(_Data, InnerKey, m_SpotInnerAngle, _Error) ||
      !ReadFloat(_Data, OuterKey, m_SpotOuterAngle, _Error))
  {
    return false;
  }

  const auto Option = std::find_if(LightTypeOptions.begin(), LightTypeOptions.end(), [&TypeName](const TLightTypeOption & _Option)
  {
    return TypeName == _Option.Key;
  });

  if (Option == LightTypeOptions.end())
  {
    _Error = "Unknown light type '" + TypeName + "'.";
    return false;
  }

  m_LightType = Option->LightType;
  return true;
}

//
// Interface
//

ELightType CLightNode::GetLightType() const noexcept
{
  return m_LightType;
}

void CLightNode::SetLightType(ELightType _LightType) noexcept
{
  m_LightType = _LightType;
}

const Ogre::ColourValue & CLightNode::GetColour() const noexcept
{
  return m_Colour;
}

float CLightNode::GetIntensity() const noexcept
{
  return m_Intensity;
}

float CLightNode::GetRange() const noexcept
{
  return m_Range;
}

float CLightNode::GetSpotInnerAngle() const noexcept
{
  return m_SpotInnerAngle;
}

float CLightNode::GetSpotOuterAngle() const noexcept
{
  return m_SpotOuterAngle;
}

} // namespace editor
