#include "SceneNodePropertySource.h"
#include "editor/inspector/PropertySchema.h"
#include "modules/scene/SceneDocument.h"
#include "modules/scene/SceneInstance.h"

#include <utility>

namespace editor
{

namespace
{

const std::string PositionIdentifier = "Transform.Position";
const std::string RotationIdentifier = "Transform.Rotation";
const std::string ScaleIdentifier    = "Transform.Scale";
const std::string MeshIdentifier     = "Render.Mesh";

TPropertySchema MakeNodeSchema()
{
  TPropertyDesc Transform{"Transform", "Transform", EPropertyType::Group};
  Transform.Children.push_back({PositionIdentifier, "Position", EPropertyType::Vector3,  {.Speed = 0.1f,  .Format = "%.2f"}});
  Transform.Children.push_back({RotationIdentifier, "Rotation", EPropertyType::Rotation, {.Speed = 0.5f,  .Format = "%.1f"}});
  Transform.Children.push_back({ScaleIdentifier,    "Scale",    EPropertyType::Vector3,  {.Speed = 0.01f, .Format = "%.3f"}});

  TPropertyDesc Render{"Render", "Render", EPropertyType::Group};
  Render.Children.push_back({MeshIdentifier, "Mesh", EPropertyType::Asset, {.AssetType = "mesh"}});

  return {"scene.node", {std::move(Transform), std::move(Render)}};
}

bool ApplyValue(TSceneTransform & _Transform, const std::string & _Identifier, const TPropertyValue & _Value)
{
  const auto * Vector      = std::get_if<Ogre::Vector3>(&_Value);
  const auto * Orientation = std::get_if<Ogre::Quaternion>(&_Value);

  if (_Identifier == PositionIdentifier && Vector)
    _Transform.Position = *Vector;
  else
  if (_Identifier == RotationIdentifier && Orientation)
    _Transform.Orientation = *Orientation;
  else
  if (_Identifier == ScaleIdentifier && Vector)
    _Transform.Scale = *Vector;
  else
    return false;

  return true;
}

} // namespace

//
// Constructor
//

CSceneNodePropertySource::CSceneNodePropertySource(std::shared_ptr<CSceneDocument> _Document):
  m_Document(std::move(_Document))
{
  // Empty
}

//
// IPropertySource
//

std::string CSceneNodePropertySource::GetTitle() const
{
  return m_Document->GetSelection();
}

const TPropertySchema & CSceneNodePropertySource::GetSchema() const
{
  static const TPropertySchema Schema = MakeNodeSchema();
  return Schema;
}

TPropertyValue CSceneNodePropertySource::GetValue(const std::string & _Identifier) const
{
  const std::string Selection = m_Document->GetSelection();
  if (Selection.empty())
    return {};

  const auto & Properties = m_Document->GetInstance().GetNode(Selection).Properties;
  const auto & Transform  = Properties.Transform;

  if (_Identifier == PositionIdentifier)
    return Transform.Position;

  if (_Identifier == RotationIdentifier)
    return Transform.Orientation;

  if (_Identifier == ScaleIdentifier)
    return Transform.Scale;

  if (_Identifier == MeshIdentifier)
    return Properties.MeshName.value_or(std::string());

  return {};
}

void CSceneNodePropertySource::Preview(const std::string & _Identifier, const TPropertyValue & _Value)
{
  const std::string Selection = m_Document->GetSelection();
  if (Selection.empty())
    return;

  auto Properties = m_Document->GetInstance().GetNode(Selection).Properties;

  if (_Identifier == MeshIdentifier)
  {
    PreviewMesh(Selection, Properties, _Value);
    return;
  }

  if (ApplyValue(Properties.Transform, _Identifier, _Value))
    m_Document->PreviewProperties(Selection, Properties, "Edit Transform");
}

void CSceneNodePropertySource::Commit()
{
  m_Document->CommitPreview();
}

//
// Implementation
//

void CSceneNodePropertySource::PreviewMesh(const std::string & _Name, TSceneNodeProperties & _Properties, const TPropertyValue & _Value)
{
  const auto * MeshName = std::get_if<std::string>(&_Value);
  if (!MeshName)
    return;

  _Properties.MeshName = MeshName->empty() ? std::nullopt : std::optional<std::string>(*MeshName);

  m_Document->PreviewProperties(_Name, _Properties, "Change Mesh");
}

} // namespace editor
