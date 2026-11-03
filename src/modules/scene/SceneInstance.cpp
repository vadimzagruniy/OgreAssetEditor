#include "SceneInstance.h"
#include "SceneSerializer.h"
#include "editor/assets/AssetRecord.h"
#include "editor/workspaces/AssetProperty.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <utility>

namespace editor
{

namespace
{

std::string GetNameStem(const std::string & _Name)
{
  const auto Separator = _Name.rfind('_');
  if (Separator == std::string::npos || Separator + 1 == _Name.size())
    return _Name;

  const bool HasNumberSuffix = std::all_of(_Name.begin() + Separator + 1, _Name.end(), [](unsigned char _Character)
  {
    return std::isdigit(_Character) != 0;
  });

  return HasNumberSuffix ? _Name.substr(0, Separator) : _Name;
}

} // namespace

//
// Constructor/Destructor
//

CSceneInstance::CSceneInstance(TAssetRecord _Record):
  CAssetInstance(std::move(_Record), "Scene")
{
  // Empty
}

CSceneInstance::~CSceneInstance() = default;

//
// CAssetInstance
//

std::vector<TAssetProperty> CSceneInstance::GetProperties() const
{
  const auto MeshCount = std::count_if(m_Nodes.begin(), m_Nodes.end(), [](const auto & _Entry)
  {
    return _Entry.second.Properties.MeshName.has_value();
  });

  return {{"Nodes",  std::to_string(m_Nodes.size())},
          {"Meshes", std::to_string(MeshCount)     }};
}

//
// Interface
//

bool CSceneInstance::HasNode(const std::string & _Name) const
{
  return m_Nodes.contains(_Name);
}

const TSceneNode & CSceneInstance::GetNode(const std::string & _Name) const
{
  const auto NodeIterator = m_Nodes.find(_Name);
  assert(NodeIterator != m_Nodes.end() && "Scene node does not exist");

  return NodeIterator->second;
}

const std::vector<std::string> & CSceneInstance::GetRootNodes() const noexcept
{
  return m_RootNodes;
}

const std::vector<std::string> & CSceneInstance::GetChildren(const std::string & _ParentName) const
{
  return _ParentName.empty() ? m_RootNodes : GetNode(_ParentName).Children;
}

std::string CSceneInstance::MakeUniqueName(const std::string & _BaseName) const
{
  if (!_BaseName.empty() && !HasNode(_BaseName))
    return _BaseName;

  const std::string Stem = _BaseName.empty() ? "Node" : GetNameStem(_BaseName);

  for (std::size_t Number = 1;; ++Number)
  {
    std::string Candidate = Stem + "_" + std::to_string(Number);
    if (!HasNode(Candidate))
      return Candidate;
  }
}

bool CSceneInstance::InsertNode(const std::string & _Name, const std::string & _ParentName, std::size_t _Index, TSceneNodeProperties _Properties, std::string & _Error)
{
  if (!CheckNewName(_Name, _Error) || !CheckParent(_ParentName, _Error))
    return false;

  auto & Siblings = GetSiblings(_ParentName);
  if (_Index > Siblings.size())
  {
    _Error = "Cannot insert node '" + _Name + "': position is out of range.";
    return false;
  }

  Siblings.insert(Siblings.begin() + static_cast<std::ptrdiff_t>(_Index), _Name);
  m_Nodes.emplace(_Name, TSceneNode{_Name, _ParentName, {}, std::move(_Properties)});

  return true;
}

bool CSceneInstance::RemoveNode(const std::string & _Name, std::string & _Error)
{
  if (!HasNode(_Name))
  {
    _Error = "Cannot remove node '" + _Name + "': it does not exist.";
    return false;
  }

  const auto & Node = GetNode(_Name);
  if (!Node.Children.empty())
  {
    _Error = "Cannot remove node '" + _Name + "': it still has children.";
    return false;
  }

  std::erase(GetSiblings(Node.ParentName), _Name);
  m_Nodes.erase(_Name);

  return true;
}

bool CSceneInstance::MoveNode(const std::string & _Name, const std::string & _NewParentName, std::size_t _Index, std::string & _Error)
{
  if (!HasNode(_Name))
  {
    _Error = "Cannot move node '" + _Name + "': it does not exist.";
    return false;
  }

  if (!CheckParent(_NewParentName, _Error))
    return false;

  if (!_NewParentName.empty() && IsSameOrDescendant(_NewParentName, _Name))
  {
    _Error = "Cannot move node '" + _Name + "' into itself or its descendant.";
    return false;
  }

  auto & Node = m_Nodes.find(_Name)->second;
  std::erase(GetSiblings(Node.ParentName), _Name);

  auto & NewSiblings = GetSiblings(_NewParentName);
  NewSiblings.insert(NewSiblings.begin() + static_cast<std::ptrdiff_t>(std::min(_Index, NewSiblings.size())), _Name);
  Node.ParentName = _NewParentName;

  return true;
}

bool CSceneInstance::RenameNode(const std::string & _Name, const std::string & _NewName, std::string & _Error)
{
  if (!HasNode(_Name))
  {
    _Error = "Cannot rename node '" + _Name + "': it does not exist.";
    return false;
  }

  if (_Name == _NewName)
    return true;

  if (!CheckNewName(_NewName, _Error))
    return false;

  auto NodeHandle = m_Nodes.extract(_Name);
  NodeHandle.key()         = _NewName;
  NodeHandle.mapped().Name = _NewName;

  auto & Node = m_Nodes.insert(std::move(NodeHandle)).position->second;

  auto & Siblings = GetSiblings(Node.ParentName);
  std::replace(Siblings.begin(), Siblings.end(), _Name, _NewName);

  for (const auto & ChildName : Node.Children)
    m_Nodes.find(ChildName)->second.ParentName = _NewName;

  return true;
}

bool CSceneInstance::SetProperties(const std::string & _Name, TSceneNodeProperties _Properties, std::string & _Error)
{
  const auto NodeIterator = m_Nodes.find(_Name);
  if (NodeIterator == m_Nodes.end())
  {
    _Error = "Cannot change node '" + _Name + "': it does not exist.";
    return false;
  }

  NodeIterator->second.Properties = std::move(_Properties);

  return true;
}

bool CSceneInstance::Save(std::string & _Error) const
{
  std::ofstream File(GetRecord().AbsolutePath, std::ios::binary | std::ios::trunc);
  File << WriteScene(*this);
  File.close();

  if (!File)
  {
    _Error = "Could not write " + GetRecord().RelativePath.generic_string();
    return false;
  }

  return true;
}

//
// Implementation
//

std::vector<std::string> & CSceneInstance::GetSiblings(const std::string & _ParentName)
{
  if (_ParentName.empty())
    return m_RootNodes;

  const auto ParentIterator = m_Nodes.find(_ParentName);
  assert(ParentIterator != m_Nodes.end() && "Scene node parent does not exist");

  return ParentIterator->second.Children;
}

bool CSceneInstance::CheckNewName(const std::string & _Name, std::string & _Error) const
{
  if (_Name.empty())
  {
    _Error = "Scene node name cannot be empty.";
    return false;
  }

  if (HasNode(_Name))
  {
    _Error = "Scene already has a node named '" + _Name + "'.";
    return false;
  }

  return true;
}

bool CSceneInstance::CheckParent(const std::string & _ParentName, std::string & _Error) const
{
  if (_ParentName.empty() || HasNode(_ParentName))
    return true;

  _Error = "Scene has no parent node named '" + _ParentName + "'.";
  return false;
}

bool CSceneInstance::IsSameOrDescendant(const std::string & _Name, const std::string & _Ancestor) const
{
  for (std::string Current = _Name; !Current.empty(); Current = GetNode(Current).ParentName)
  {
    if (Current == _Ancestor)
      return true;
  }

  return false;
}

} // namespace editor
