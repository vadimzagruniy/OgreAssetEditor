#include "PrimitiveNode.h"
#include "editor/inspector/PropertyDesc.h"
#include "modules/scene/SceneJsonFields.h"

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

struct TShapeOption
{
  EPrimitiveShape Shape;
  const char *    Key;
  const char *    Label;
};

constexpr std::array<TShapeOption, 3> ShapeOptions{{
  {EPrimitiveShape::Cube,   "cube",   "Cube"  },
  {EPrimitiveShape::Sphere, "sphere", "Sphere"},
  {EPrimitiveShape::Plane,  "plane",  "Plane" }
}};

const std::string ShapeIdentifier = "Primitive.Shape";
const std::string ShapeKey        = "shape";

std::size_t FindShapeIndex(EPrimitiveShape _Shape)
{
  const auto Option = std::find_if(ShapeOptions.begin(), ShapeOptions.end(), [_Shape](const TShapeOption & _Option)
  {
    return _Option.Shape == _Shape;
  });

  return static_cast<std::size_t>(std::distance(ShapeOptions.begin(), Option));
}

} // namespace

//
// Constructor
//

CPrimitiveNode::CPrimitiveNode():
  m_Shape(EPrimitiveShape::Cube)
{
  // Empty
}

//
// CSceneNode
//

std::string_view CPrimitiveNode::GetTypeId() const noexcept
{
  return TypeId;
}

void CPrimitiveNode::DescribeProperties(std::vector<TPropertyDesc> & _Properties) const
{
  CSceneNode::DescribeProperties(_Properties);

  std::vector<std::string> ShapeLabels;
  for (const auto & Option : ShapeOptions)
    ShapeLabels.emplace_back(Option.Label);

  TPropertyDesc Primitive{"Primitive", "Primitive", EPropertyType::Group};
  Primitive.Children.push_back({ShapeIdentifier, "Shape", EPropertyType::Enum, {.Options = std::move(ShapeLabels)}});

  _Properties.push_back(std::move(Primitive));
}

TPropertyValue CPrimitiveNode::GetValue(const std::string & _Identifier) const
{
  if (_Identifier == ShapeIdentifier)
    return FindShapeIndex(m_Shape);

  return CSceneNode::GetValue(_Identifier);
}

bool CPrimitiveNode::SetValue(const std::string & _Identifier, const TPropertyValue & _Value)
{
  if (_Identifier != ShapeIdentifier)
    return CSceneNode::SetValue(_Identifier, _Value);

  const auto * OptionIndex = std::get_if<std::size_t>(&_Value);
  if (!OptionIndex || *OptionIndex >= ShapeOptions.size())
    return false;

  m_Shape = ShapeOptions[*OptionIndex].Shape;
  return true;
}

void CPrimitiveNode::OnWriteData(TSceneJson & _Data) const
{
  _Data[ShapeKey] = ShapeOptions[FindShapeIndex(m_Shape)].Key;
}

bool CPrimitiveNode::OnReadData(const TSceneJson & _Data, std::string & _Error)
{
  std::string ShapeName = ShapeOptions[FindShapeIndex(m_Shape)].Key;
  if (!ReadString(_Data, ShapeKey, ShapeName, _Error))
    return false;

  const auto Option = std::find_if(ShapeOptions.begin(), ShapeOptions.end(), [&ShapeName](const TShapeOption & _Option)
  {
    return ShapeName == _Option.Key;
  });

  if (Option == ShapeOptions.end())
  {
    _Error = "Unknown primitive shape '" + ShapeName + "'.";
    return false;
  }

  m_Shape = Option->Shape;
  return true;
}

//
// Interface
//

EPrimitiveShape CPrimitiveNode::GetShape() const noexcept
{
  return m_Shape;
}

void CPrimitiveNode::SetShape(EPrimitiveShape _Shape) noexcept
{
  m_Shape = _Shape;
}

} // namespace editor
