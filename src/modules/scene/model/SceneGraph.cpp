#include "SceneGraph.h"

#include <algorithm>
#include <cassert>
#include <cctype>
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
// Interface
//

std::size_t CSceneGraph::GetNodeCount() const noexcept
{
  return m_Nodes.size();
}

std::size_t CSceneGraph::GetMeshNodeCount() const
{
  return static_cast<std::size_t>(std::count_if(m_Nodes.begin(), m_Nodes.end(), [](const auto & _Entry)
  {
    return _Entry.second.Properties.MeshName.has_value();
  }));
}

bool CSceneGraph::HasNode(const std::string & _Name) const
{
  return m_Nodes.contains(_Name);
}

const TSceneNode & CSceneGraph::GetNode(const std::string & _Name) const
{
  const auto NodeIterator = m_Nodes.find(_Name);
  assert(NodeIterator != m_Nodes.end() && "Scene node does not exist");

  return NodeIterator->second;
}

const std::vector<std::string> & CSceneGraph::GetRootNodes() const noexcept
{
  return m_RootNodes;
}

const std::vector<std::string> & CSceneGraph::GetChildren(const std::string & _ParentName) const
{
  return _ParentName.empty() ? m_RootNodes : GetNode(_ParentName).Children;
}

std::string CSceneGraph::MakeUniqueName(const std::string & _BaseName) const
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

std::size_t CSceneGraph::GetSiblingIndex(const std::string & _Name) const
{
  const auto & Siblings = GetChildren(GetNode(_Name).ParentName);
  return static_cast<std::size_t>(std::find(Siblings.begin(), Siblings.end(), _Name) - Siblings.begin());
}

bool CSceneGraph::IsSameOrDescendant(const std::string & _Name, const std::string & _Ancestor) const
{
  for (std::string Current = _Name; !Current.empty(); Current = GetNode(Current).ParentName)
  {
    if (Current == _Ancestor)
      return true;
  }

  return false;
}

Ogre::Affine3 CSceneGraph::GetWorldTransform(const std::string & _Name) const
{
  Ogre::Affine3 World = Ogre::Affine3::IDENTITY;

  for (std::string Current = _Name; !Current.empty(); Current = GetNode(Current).ParentName)
  {
    const auto & Transform = GetNode(Current).Properties.Transform;

    Ogre::Affine3 Local;
    Local.makeTransform(Transform.Position, Transform.Scale, Transform.Orientation);

    World = Local * World;
  }

  return World;
}

TSceneSubtree CSceneGraph::CaptureSubtree(const std::string & _Name) const
{
  TSceneSubtree Subtree;
  Subtree.Index = GetSiblingIndex(_Name);

  CaptureNode(_Name, Subtree);

  return Subtree;
}

bool CSceneGraph::InsertSubtree(const TSceneSubtree & _Subtree, std::string & _Error)
{
  for (std::size_t NodeIndex = 0; NodeIndex < _Subtree.Nodes.size(); ++NodeIndex)
  {
    const auto &      Node  = _Subtree.Nodes[NodeIndex];
    const std::size_t Index = NodeIndex == 0 ? _Subtree.Index : GetChildren(Node.ParentName).size();

    if (!InsertNode(Node.Name, Node.ParentName, Index, Node.Properties, _Error))
      return false;
  }

  return true;
}

bool CSceneGraph::RemoveSubtree(const std::string & _Name, std::string & _Error)
{
  if (!HasNode(_Name))
  {
    _Error = "Cannot remove node '" + _Name + "': it does not exist.";
    return false;
  }

  const auto Children = GetNode(_Name).Children;
  for (const auto & ChildName : Children)
  {
    if (!RemoveSubtree(ChildName, _Error))
      return false;
  }

  return RemoveNode(_Name, _Error);
}

bool CSceneGraph::InsertNode(const std::string & _Name, const std::string & _ParentName, std::size_t _Index, TSceneNodeProperties _Properties, std::string & _Error)
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

  MarkDirty(_Name);

  return true;
}

bool CSceneGraph::RemoveNode(const std::string & _Name, std::string & _Error)
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

  MarkDirty(_Name);

  return true;
}

bool CSceneGraph::MoveNode(const std::string & _Name, const std::string & _NewParentName, std::size_t _Index, std::string & _Error)
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

  MarkDirty(_Name);

  return true;
}

bool CSceneGraph::RenameNode(const std::string & _Name, const std::string & _NewName, std::string & _Error)
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
  {
    m_Nodes.find(ChildName)->second.ParentName = _NewName;
    MarkDirty(ChildName);
  }

  MarkDirty(_Name);
  MarkDirty(_NewName);

  return true;
}

bool CSceneGraph::SetProperties(const std::string & _Name, TSceneNodeProperties _Properties, std::string & _Error)
{
  const auto NodeIterator = m_Nodes.find(_Name);
  if (NodeIterator == m_Nodes.end())
  {
    _Error = "Cannot change node '" + _Name + "': it does not exist.";
    return false;
  }

  NodeIterator->second.Properties = std::move(_Properties);

  MarkDirty(_Name);

  return true;
}

std::unordered_set<std::string> CSceneGraph::TakeDirtyNodes()
{
  return std::exchange(m_DirtyNodes, {});
}

//
// Implementation
//

void CSceneGraph::MarkDirty(const std::string & _Name)
{
  m_DirtyNodes.insert(_Name);
}

std::vector<std::string> & CSceneGraph::GetSiblings(const std::string & _ParentName)
{
  if (_ParentName.empty())
    return m_RootNodes;

  const auto ParentIterator = m_Nodes.find(_ParentName);
  assert(ParentIterator != m_Nodes.end() && "Scene node parent does not exist");

  return ParentIterator->second.Children;
}

bool CSceneGraph::CheckNewName(const std::string & _Name, std::string & _Error) const
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

bool CSceneGraph::CheckParent(const std::string & _ParentName, std::string & _Error) const
{
  if (_ParentName.empty() || HasNode(_ParentName))
    return true;

  _Error = "Scene has no parent node named '" + _ParentName + "'.";
  return false;
}

void CSceneGraph::CaptureNode(const std::string & _Name, TSceneSubtree & _Subtree) const
{
  const auto & Node = GetNode(_Name);
  _Subtree.Nodes.push_back({Node.Name, Node.ParentName, Node.Properties});

  for (const auto & ChildName : Node.Children)
    CaptureNode(ChildName, _Subtree);
}

} // namespace editor
