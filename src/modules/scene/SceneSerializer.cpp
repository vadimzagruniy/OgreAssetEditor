#include "SceneSerializer.h"
#include "SceneInstance.h"
#include "model/SceneNodeProperties.h"

#include <nlohmann/json.hpp>

#include <array>
#include <cassert>
#include <cstddef>

namespace editor
{

namespace
{

constexpr std::int64_t SceneFormatVersion = 1;

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

  for (std::size_t Index = 0; Index < TCount; ++Index)
  {
    const auto & Element = (*Field)[Index];
    if (!Element.is_number())
    {
      _Error = "'" + _Key + "' must contain only numbers.";
      return false;
    }

    _Numbers[Index] = Element.get<float>();
  }

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

bool ReadNode(const TSceneJson & _Json, const std::string & _ParentName, CSceneInstance & _Scene, std::string & _Error)
{
  if (!_Json.is_object())
  {
    _Error = "Scene node must be a JSON object.";
    return false;
  }

  const auto NameField = _Json.find("name");
  if (NameField == _Json.end() || !NameField->is_string())
  {
    _Error = "Scene node has no 'name' string.";
    return false;
  }

  const auto & Name = NameField->get_ref<const std::string &>();

  TSceneNodeProperties Properties;
  if (!PropertiesFromJson(_Json, Properties, _Error))
  {
    _Error = "Node '" + Name + "': " + _Error;
    return false;
  }

  if (!_Scene.InsertNode(Name, _ParentName, _Scene.GetChildren(_ParentName).size(), std::move(Properties), _Error))
    return false;

  const auto ChildrenField = _Json.find("children");
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

TSceneJson WriteNode(const CSceneInstance & _Scene, const std::string & _Name)
{
  const auto & Node = _Scene.GetNode(_Name);

  TSceneJson Json = TSceneJson::object();
  Json["name"] = Node.Name;

  const TSceneJson Properties = PropertiesToJson(Node.Properties);
  for (const auto & Property : Properties.items())
    Json[Property.key()] = Property.value();

  if (Node.Children.empty())
    return Json;

  auto & Children = Json["children"] = TSceneJson::array();
  for (const auto & ChildName : Node.Children)
    Children.push_back(WriteNode(_Scene, ChildName));

  return Json;
}

} // namespace

bool ReadScene(const std::string & _Text, CSceneInstance & _Scene, std::string & _Error)
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

std::string WriteScene(const CSceneInstance & _Scene)
{
  TSceneJson Root = TSceneJson::object();
  Root["version"] = SceneFormatVersion;

  auto & Nodes = Root["nodes"] = TSceneJson::array();
  for (const auto & Name : _Scene.GetRootNodes())
    Nodes.push_back(WriteNode(_Scene, Name));

  return Root.dump(2, ' ', false, TSceneJson::error_handler_t::replace) + "\n";
}

TSceneJson PropertiesToJson(const TSceneNodeProperties & _Properties)
{
  const auto & Transform = _Properties.Transform;

  TSceneJson Json = TSceneJson::object();
  Json["position"]    = {Transform.Position.x, Transform.Position.y, Transform.Position.z};
  Json["orientation"] = {Transform.Orientation.w, Transform.Orientation.x, Transform.Orientation.y, Transform.Orientation.z};
  Json["scale"]       = {Transform.Scale.x, Transform.Scale.y, Transform.Scale.z};

  if (_Properties.MeshName)
    Json["mesh"] = *_Properties.MeshName;

  return Json;
}

bool PropertiesFromJson(const TSceneJson & _Json, TSceneNodeProperties & _Properties, std::string & _Error)
{
  auto & Transform = _Properties.Transform;

  if (!ReadVector3(_Json, "position", Transform.Position, _Error) ||
      !ReadQuaternion(_Json, "orientation", Transform.Orientation, _Error) ||
      !ReadVector3(_Json, "scale", Transform.Scale, _Error))
  {
    return false;
  }

  const auto MeshField = _Json.find("mesh");
  if (MeshField == _Json.end())
  {
    _Properties.MeshName.reset();
    return true;
  }

  if (!MeshField->is_string() || MeshField->get_ref<const std::string &>().empty())
  {
    _Error = "'mesh' must be a non-empty string.";
    return false;
  }

  _Properties.MeshName = MeshField->get<std::string>();

  return true;
}

} // namespace editor
