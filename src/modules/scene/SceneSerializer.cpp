#include "SceneSerializer.h"
#include "editor/json/Json.h"
#include "editor/json/JsonFields.h"
#include "model/SceneAnimation.h"
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

constexpr std::int64_t SceneFormatVersion = 3;

const std::string NameKey        = "name";
const std::string TypeKey        = "type";
const std::string PositionKey    = "position";
const std::string OrientationKey = "orientation";
const std::string ScaleKey       = "scale";
const std::string ChildrenKey    = "children";
const std::string LegacyMeshKey  = "mesh";
const std::string AnimationsKey  = "animations";
const std::string PathKey        = "path";
const std::string BindingsKey    = "bindings";

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

std::string ReadTypeId(const TJson & _Json, std::string & _Error)
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

bool ReadSnapshot(const TJson & _Json, TSceneNodeSnapshot & _Snapshot, std::string & _Error)
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

bool ReadNode(const TJson & _Json, const std::string & _ParentName, CSceneGraph & _Scene, std::string & _Error)
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

bool ReadNodes(const TJson & _Root, CSceneGraph & _Scene, std::string & _Error)
{
  const auto NodesField = _Root.find("nodes");
  if (NodesField == _Root.end())
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

bool ReadBindings(const TJson & _Json, TSceneAnimation & _Animation, std::string & _Error)
{
  const auto BindingsField = _Json.find(BindingsKey);
  if (BindingsField == _Json.end())
    return true;

  if (!BindingsField->is_object())
  {
    _Error = "Animation 'bindings' must be an object.";
    return false;
  }

  for (const auto & Binding : BindingsField->items())
  {
    if (!Binding.value().is_string())
    {
      _Error = "Animation binding '" + Binding.key() + "' must be a node name string.";
      return false;
    }

    _Animation.Bindings[Binding.key()] = Binding.value().get<std::string>();
  }

  return true;
}

bool ReadAnimations(const TJson & _Root, std::vector<TSceneAnimation> & _Animations, std::string & _Error)
{
  const auto AnimationsField = _Root.find(AnimationsKey);
  if (AnimationsField == _Root.end())
    return true;

  if (!AnimationsField->is_array())
  {
    _Error = "Scene 'animations' must be an array.";
    return false;
  }

  for (const auto & AnimationJson : *AnimationsField)
  {
    TSceneAnimation Animation;

    if (!AnimationJson.is_object() || !ReadString(AnimationJson, PathKey, Animation.Path, _Error) || Animation.Path.empty())
    {
      _Error = _Error.empty() ? "Scene animation must be an object with a 'path' string." : _Error;
      return false;
    }

    if (!ReadBindings(AnimationJson, Animation, _Error))
    {
      _Error = "Animation '" + Animation.Path + "': " + _Error;
      return false;
    }

    _Animations.push_back(std::move(Animation));
  }

  return true;
}

TJson WriteAnimation(const TSceneAnimation & _Animation)
{
  TJson Json = TJson::object();
  Json[PathKey] = _Animation.Path;

  if (_Animation.Bindings.empty())
    return Json;

  auto & Bindings = Json[BindingsKey] = TJson::object();
  for (const auto & [Slot, NodeName] : _Animation.Bindings)
    Bindings[Slot] = NodeName;

  return Json;
}

TJson WriteNode(const CSceneGraph & _Scene, const std::string & _Name)
{
  const auto & Node     = _Scene.GetNode(_Name);
  const auto   Snapshot = Node.CaptureSnapshot();

  TJson Json = TJson::object();
  Json[NameKey]        = Node.GetName();
  Json[TypeKey]        = Snapshot.TypeId;
  Json[PositionKey]    = WriteVector3(Snapshot.Transform.Position);
  Json[OrientationKey] = WriteQuaternion(Snapshot.Transform.Orientation);
  Json[ScaleKey]       = WriteVector3(Snapshot.Transform.Scale);

  for (const auto & Field : Snapshot.Data.items())
    Json[Field.key()] = Field.value();

  if (Node.GetChildren().empty())
    return Json;

  auto & Children = Json[ChildrenKey] = TJson::array();
  for (const auto & ChildName : Node.GetChildren())
    Children.push_back(WriteNode(_Scene, ChildName));

  return Json;
}

} // namespace

bool ReadScene(const std::string & _Text, CSceneGraph & _Scene, std::vector<TSceneAnimation> & _Animations, std::string & _Error)
{
  assert(_Scene.GetRootNodes().empty() && "Scene must be empty before reading");

  const auto Root = TJson::parse(_Text, nullptr, false);
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

  return ReadNodes(Root, _Scene, _Error) && ReadAnimations(Root, _Animations, _Error);
}

std::string WriteScene(const CSceneGraph & _Scene, const std::vector<TSceneAnimation> & _Animations)
{
  TJson Root = TJson::object();
  Root["version"] = SceneFormatVersion;

  auto & Nodes = Root["nodes"] = TJson::array();
  for (const auto & Name : _Scene.GetRootNodes())
    Nodes.push_back(WriteNode(_Scene, Name));

  if (!_Animations.empty())
  {
    auto & Animations = Root[AnimationsKey] = TJson::array();
    for (const auto & Animation : _Animations)
      Animations.push_back(WriteAnimation(Animation));
  }

  return Root.dump(2, ' ', false, TJson::error_handler_t::replace) + "\n";
}

} // namespace editor
