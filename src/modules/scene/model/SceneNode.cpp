#include "SceneNode.h"
#include "SceneNodeSnapshot.h"
#include "editor/inspector/PropertyDesc.h"

#include <utility>

namespace editor
{

namespace
{

const std::string PositionIdentifier = "Transform.Position";
const std::string RotationIdentifier = "Transform.Rotation";
const std::string ScaleIdentifier    = "Transform.Scale";

} // namespace

//
// Constructor/Destructor
//

CSceneNode::CSceneNode() = default;

CSceneNode::~CSceneNode() = default;

//
// Interface
//

const std::string & CSceneNode::GetName() const noexcept
{
  return m_Name;
}

const std::string & CSceneNode::GetParentName() const noexcept
{
  return m_ParentName;
}

const std::vector<std::string> & CSceneNode::GetChildren() const noexcept
{
  return m_Children;
}

const TSceneTransform & CSceneNode::GetTransform() const noexcept
{
  return m_Transform;
}

void CSceneNode::SetTransform(const TSceneTransform & _Transform) noexcept
{
  m_Transform = _Transform;
}

std::string_view CSceneNode::GetTypeId() const noexcept
{
  return TypeId;
}

TSceneNodeSnapshot CSceneNode::CaptureSnapshot() const
{
  TSceneNodeSnapshot Snapshot{std::string(GetTypeId()), m_Transform};
  OnWriteData(Snapshot.Data);

  return Snapshot;
}

bool CSceneNode::ApplySnapshot(const TSceneNodeSnapshot & _Snapshot, std::string & _Error)
{
  if (_Snapshot.TypeId != GetTypeId())
  {
    _Error = "Node type '" + _Snapshot.TypeId + "' does not match '" + std::string(GetTypeId()) + "'.";
    return false;
  }

  if (!OnReadData(_Snapshot.Data, _Error))
    return false;

  m_Transform = _Snapshot.Transform;

  return true;
}

void CSceneNode::DescribeProperties(std::vector<TPropertyDesc> & _Properties) const
{
  TPropertyDesc Transform{"Transform", "Transform", EPropertyType::Group};
  Transform.Children.push_back({PositionIdentifier, "Position", EPropertyType::Vector3,  {.Speed = 0.1f,  .Format = "%.2f"}});
  Transform.Children.push_back({RotationIdentifier, "Rotation", EPropertyType::Rotation, {.Speed = 0.5f,  .Format = "%.1f"}});
  Transform.Children.push_back({ScaleIdentifier,    "Scale",    EPropertyType::Vector3,  {.Speed = 0.01f, .Format = "%.3f"}});

  _Properties.push_back(std::move(Transform));
}

TPropertyValue CSceneNode::GetValue(const std::string & _Identifier) const
{
  if (_Identifier == PositionIdentifier)
    return m_Transform.Position;

  if (_Identifier == RotationIdentifier)
    return m_Transform.Orientation;

  if (_Identifier == ScaleIdentifier)
    return m_Transform.Scale;

  return {};
}

bool CSceneNode::SetValue(const std::string & _Identifier, const TPropertyValue & _Value)
{
  const auto * Vector      = std::get_if<Ogre::Vector3>(&_Value);
  const auto * Orientation = std::get_if<Ogre::Quaternion>(&_Value);

  if (_Identifier == PositionIdentifier && Vector)
    m_Transform.Position = *Vector;
  else
  if (_Identifier == RotationIdentifier && Orientation)
    m_Transform.Orientation = *Orientation;
  else
  if (_Identifier == ScaleIdentifier && Vector)
    m_Transform.Scale = *Vector;
  else
    return false;

  return true;
}

//
// Events
//

void CSceneNode::OnWriteData(TJson & /*_Data*/) const
{
  // Empty
}

bool CSceneNode::OnReadData(const TJson & /*_Data*/, std::string & /*_Error*/)
{
  return true;
}

} // namespace editor
