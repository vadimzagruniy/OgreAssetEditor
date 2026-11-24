#include "SceneSerializer.h"
#include "SceneJson.h"
#include "SceneJsonFields.h"
#include "model/SceneGraph.h"
#include "model/SceneNode.h"
#include "model/SceneNodeSnapshot.h"
#include "model/nodes/MeshNode.h"

#include <nlohmann/json.hpp>

#include <array>
#include <cassert>
#include <string_view>

namespace editor
{

namespace
{

constexpr std::int64_t SceneFormatVersion = 2;

const std::string NameKey        = "name";
const std::string TypeKey        = "type";
const std::string PositionKey    = "position";
const std::string OrientationKey = "orientation";
const std::string ScaleKey       = "scale";
const std::string ChildrenKey    = "children";
const std::string LegacyMeshKey  = "mesh";

constexpr std::array<std::string_view, 6> ReservedKeys{"name", "type", "position", "orientation", "scale", "children"};

bool IsReservedKey(const std::string & _Key)
{
  for (const auto ReservedKey : ReservedKeys)
  {
    if (_Key == ReservedKey)
      return true;
  }

  return false;
}

std::string ReadTypeId(const TSceneJson & _Json, std::string & _Error)
{
  const auto TypeField = _Json.find(TypeKey);
  if (TypeField == _Json.end())
    return std::string(_Json.contains(LegacyMeshKey) ? CMeshNode::TypeId : CSceneNode::TypeId);

  if (!TypeField->is_string() || TypeField->get_ref<const std::string &>().empty())
  {
    _Error = "'type' must be a non-empty string.";
    return {};
  }

  return TypeField->get<std::string>();
}

bool ReadSnapshot(const TSceneJson & _Json, TSceneNodeSnapshot & _Snapshot, std::string & _Error)
{
  _Snapshot.TypeId = ReadTypeId(_Json, _Error);
  if (_Snapshot.TypeId.empty())
    return false;

  auto & Transform = _Snapshot.Transform;

  if (!ReadVector3(_Json, PositionKey, Transform.Position, _Error) ||
      !ReadQuaternion(_Json, OrientationKey, Transform.Orientation, _Error) ||
      !ReadVector3(_Json, ScaleKey, Transform.Scale, _Error))
  {
    return false;
  }

  for (const auto & Field : _Json.items())
  {
    if (!IsReservedKey(Field.key()))
      _Snapshot.Data[Field.key()] = Field.value();
  }

  return true;
}

bool ReadNode(const TSceneJson & _Json, const std::string & _ParentName, CSceneGraph & _Scene, std::string & _Error)
{
  if (!_Json.is_object())
  {
    _Error = "Scene node must be a JSON object.";
    return false;
  }

  const auto NameField = _Json.find(NameKey);
  if (NameField == _Json.end() || !NameField->is_string())
  {
    _Error = "Scene node has no 'name' string.";
    return false;
  }

  const auto & Name = NameField->get_ref<const std::string &>();

  TSceneNodeSnapshot Snapshot;
  if (!ReadSnapshot(_Json, Snapshot, _Error) || !_Scene.InsertNode(Name, _ParentName, _Scene.GetChildren(_ParentName).size(), Snapshot, _Error))
  {
    _Error = "Node '" + Name + "': " + _Error;
    return false;
  }

  const auto ChildrenField = _Json.find(ChildrenKey);
  if (ChildrenField == _Json.end())
    return true;

  if (!ChildrenField->is_array())
  {
    _Error = "Node '" + Name + "': 'children' must be an array.";
    return false;
  }

  for (const auto & Child : *ChildrenField)
  {
    if (!ReadNode(Child, Name, _Scene, _Error))
      return false;
  }

  return true;
}

TSceneJson WriteNode(const CSceneGraph & _Scene, const std::string & _Name)
{
  const auto & Node     = _Scene.GetNode(_Name);
  const auto   Snapshot = Node.CaptureSnapshot();

  TSceneJson Json = TSceneJson::object();
  Json[NameKey]        = Node.GetName();
  Json[TypeKey]        = Snapshot.TypeId;
  Json[PositionKey]    = WriteVector3(Snapshot.Transform.Position);
  Json[OrientationKey] = WriteQuaternion(Snapshot.Transform.Orientation);
  Json[ScaleKey]       = WriteVector3(Snapshot.Transform.Scale);

  for (const auto & Field : Snapshot.Data.items())
    Json[Field.key()] = Field.value();

  if (Node.GetChildren().empty())
    return Json;

  auto & Children = Json[ChildrenKey] = TSceneJson::array();
  for (const auto & ChildName : Node.GetChildren())
    Children.push_back(WriteNode(_Scene, ChildName));

  return Json;
}

} // namespace

bool ReadScene(const std::string & _Text, CSceneGraph & _Scene, std::string & _Error)
{
  assert(_Scene.GetRootNodes().empty() && "Scene must be empty before reading");

  const auto Root = TSceneJson::parse(_Text, nullptr, false);
  if (Root.is_discarded() || !Root.is_object())
  {
    _Error = "Scene file is not a valid JSON object.";
    return false;
  }

  const auto VersionField = Root.find("version");
  if (VersionField == Root.end() || !VersionField->is_number_integer())
  {
    _Error = "Scene file has no integer 'version'.";
    return false;
  }

  const auto Version = VersionField->get<std::int64_t>();
  if (Version < 1 || Version > SceneFormatVersion)
  {
    _Error = "Scene format version " + std::to_string(Version) + " is not supported.";
    return false;
  }

  const auto NodesField = Root.find("nodes");
  if (NodesField == Root.end())
    return true;

  if (!NodesField->is_array())
  {
    _Error = "Scene 'nodes' must be an array.";
    return false;
  }

  for (const auto & Node : *NodesField)
  {
    if (!ReadNode(Node, {}, _Scene, _Error))
      return false;
  }

  return true;
}

std::string WriteScene(const CSceneGraph & _Scene)
{
  TSceneJson Root = TSceneJson::object();
  Root["version"] = SceneFormatVersion;

  auto & Nodes = Root["nodes"] = TSceneJson::array();
  for (const auto & Name : _Scene.GetRootNodes())
    Nodes.push_back(WriteNode(_Scene, Name));

  return Root.dump(2, ' ', false, TSceneJson::error_handler_t::replace) + "\n";
}

} // namespace editor
