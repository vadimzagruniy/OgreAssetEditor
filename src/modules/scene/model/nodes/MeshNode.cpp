#include "MeshNode.h"
#include "editor/inspector/PropertyDesc.h"
#include "editor/json/JsonFields.h"

#include <nlohmann/json.hpp>

#include <utility>

namespace editor
{

namespace
{

const std::string MeshIdentifier = "Render.Mesh";
const std::string MeshKey        = "mesh";

} // namespace

//
// CSceneNode
//

std::string_view CMeshNode::GetTypeId() const noexcept
{
  return TypeId;
}

void CMeshNode::DescribeProperties(std::vector<TPropertyDesc> & _Properties) const
{
  CSceneNode::DescribeProperties(_Properties);

  TPropertyDesc Render{"Render", "Render", EPropertyType::Group};
  Render.Children.push_back({MeshIdentifier, "Mesh", EPropertyType::Asset, {.AssetType = "mesh"}});

  _Properties.push_back(std::move(Render));
}

TPropertyValue CMeshNode::GetValue(const std::string & _Identifier) const
{
  if (_Identifier == MeshIdentifier)
    return m_MeshName;

  return CSceneNode::GetValue(_Identifier);
}

bool CMeshNode::SetValue(const std::string & _Identifier, const TPropertyValue & _Value)
{
  if (_Identifier != MeshIdentifier)
    return CSceneNode::SetValue(_Identifier, _Value);

  const auto * MeshName = std::get_if<std::string>(&_Value);
  if (!MeshName)
    return false;

  m_MeshName = *MeshName;
  return true;
}

void CMeshNode::OnWriteData(TJson & _Data) const
{
  if (!m_MeshName.empty())
    _Data[MeshKey] = m_MeshName;
}

bool CMeshNode::OnReadData(const TJson & _Data, std::string & _Error)
{
  std::string MeshName;
  if (!ReadString(_Data, MeshKey, MeshName, _Error))
    return false;

  m_MeshName = std::move(MeshName);
  return true;
}

//
// Interface
//

const std::string & CMeshNode::GetMeshName() const noexcept
{
  return m_MeshName;
}

void CMeshNode::SetMeshName(std::string _MeshName)
{
  m_MeshName = std::move(_MeshName);
}

} // namespace editor
