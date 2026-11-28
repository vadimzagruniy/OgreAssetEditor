#include "MeshNode.h"
#include "editor/assets/AssetReferenceJson.h"
#include "editor/inspector/PropertyDesc.h"

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
    return m_MeshRef;

  return CSceneNode::GetValue(_Identifier);
}

bool CMeshNode::SetValue(const std::string & _Identifier, const TPropertyValue & _Value)
{
  if (_Identifier != MeshIdentifier)
    return CSceneNode::SetValue(_Identifier, _Value);

  const auto * MeshRef = std::get_if<TAssetReference>(&_Value);
  if (!MeshRef)
    return false;

  m_MeshRef = *MeshRef;
  return true;
}

void CMeshNode::VisitAssetReferences(const TAssetReferenceVisitor & _Visitor)
{
  if (!m_MeshRef.IsEmpty())
    _Visitor(m_MeshRef);
}

void CMeshNode::OnWriteData(TJson & _Data) const
{
  if (!m_MeshRef.IsEmpty())
    _Data[MeshKey] = WriteAssetReference(m_MeshRef);
}

bool CMeshNode::OnReadData(const TJson & _Data, std::string & _Error)
{
  TAssetReference MeshRef;
  if (!ReadAssetReference(_Data, MeshKey, MeshRef, _Error))
    return false;

  m_MeshRef = std::move(MeshRef);
  return true;
}

//
// Interface
//

const TAssetReference & CMeshNode::GetMeshRef() const noexcept
{
  return m_MeshRef;
}

void CMeshNode::SetMeshRef(TAssetReference _MeshRef)
{
  m_MeshRef = std::move(_MeshRef);
}

} // namespace editor
