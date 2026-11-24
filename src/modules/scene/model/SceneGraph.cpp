#include "SceneGraph.h"
#include "SceneNode.h"
#include "SceneNodeSnapshot.h"
#include "SceneNodeTypes.h"

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
// Constructor/Destructor
//

CSceneGraph::CSceneGraph(std::shared_ptr<const CSceneNodeTypes> _NodeTypes):
  m_NodeTypes(std::move(_NodeTypes))
{
  // Empty
}

CSceneGraph::~CSceneGraph() = default;

//
// Interface
//

const CSceneNodeTypes & CSceneGraph::GetNodeTypes() const noexcept
{
  return *m_NodeTypes;
}

std::size_t CSceneGraph::GetNodeCount() const noexcept
{
  return m_Nodes.size();
}

bool CSceneGraph::HasNode(const std::string & _Name) const
{
  return m_Nodes.contains(_Name);
}

bool CSceneGraph::HasNodeOfType(std::string_view _TypeId) const
{
  return std::any_of(m_Nodes.begin(), m_Nodes.end(), [_TypeId](const auto & _Entry)
  {
    return _Entry.second->GetTypeId() == _TypeId;
  });
}

const CSceneNode & CSceneGraph::GetNode(const std::string & _Name) const
{
  const auto NodeIterator = m_Nodes.find(_Name);
  assert(NodeIterator != m_Nodes.end() && "Scene node does not exist");

  return *NodeIterator->second;
}

std::vector<std::string> CSceneGraph::GetNodeNames() const
{
  std::vector<std::string> Names;
  Names.reserve(m_Nodes.size());

  for (const auto & Entry : m_Nodes)
    Names.push_back(Entry.first);

  return Names;
}

const std::vector<std::string> & CSceneGraph::GetRootNodes() const noexcept
{
  return m_RootNodes;
}

const std::vector<std::string> & CSceneGraph::GetChildren(const std::string & _ParentName) const
{
  return _ParentName.empty() ? m_RootNodes : GetNode(_ParentName).GetChildren();
}

std::string CSceneGraph::MakeUniqueName(const std::string & _BaseName, const std::unordered_set<std::string> & _Reserved) const
{
  const auto IsFree = [this, &_Reserved](const std::string & _Name)
  {
    return !HasNode(_Name) && !_Reserved.contains(_Name);
  };

  if (!_BaseName.empty() && IsFree(_BaseName))
    return _BaseName;

  const std::string Stem = _BaseName.empty() ? "Node" : GetNameStem(_BaseName);

  for (std::size_t Number = 1;; ++Number)
  {
    std::string Candidate = Stem + "_" + std::to_string(Number);
    if (IsFree(Candidate))
      return Candidate;
  }
}

std::size_t CSceneGraph::GetSiblingIndex(const std::string & _Name) const
{
  const auto & Siblings = GetChildren(GetNode(_Name).GetParentName());
  return static_cast<std::size_t>(std::find(Siblings.begin(), Siblings.end(), _Name) - Siblings.begin());
}

bool CSceneGraph::IsSameOrDescendant(const std::string & _Name, const std::string & _Ancestor) const
{
  for (std::string Current = _Name; !Current.empty(); Current = GetNode(Current).GetParentName())
  {
    if (Current == _Ancestor)
      return true;
  }

  return false;
}

Ogre::Affine3 CSceneGraph::GetWorldTransform(const std::string & _Name) const
{
  Ogre::Affine3 World = Ogre::Affine3::IDENTITY;

  for (std::string Current = _Name; !Current.empty(); Current = GetNode(Current).GetParentName())
  {
    const auto & Transform = GetNode(Current).GetTransform();

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

    if (!InsertNode(Node.Name, Node.ParentName, Index, Node.Snapshot, _Error))
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

  const auto Children = GetNode(_Name).GetChildren();
  for (const auto & ChildName : Children)
  {
    if (!RemoveSubtree(ChildName, _Error))
      return false;
  }

  return RemoveNode(_Name, _Error);
}

bool CSceneGraph::InsertNode(const std::string & _Name, const std::string & _ParentName, std::size_t _Index, const TSceneNodeSnapshot & _Snapshot, std::string & _Error)
{
  if (!CheckNewName(_Name, _Error) || !CheckParent(_ParentName, _Error))
    return false;

  auto & Siblings = GetSiblings(_ParentName);
  if (_Index > Siblings.size())
  {
    _Error = "Cannot insert node '" + _Name + "': position is out of range.";
    return false;
  }

  auto Node = CreateNode(_Snapshot, _Error);
  if (!Node)
  {
    _Error = "Cannot insert node '" + _Name + "': " + _Error;
    return false;
  }

  Node->m_Name       = _Name;
  Node->m_ParentName = _ParentName;

  Siblings.insert(Siblings.begin() + static_cast<std::ptrdiff_t>(_Index), _Name);
  m_Nodes.emplace(_Name, std::move(Node));

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
  if (!Node.GetChildren().empty())
  {
    _Error = "Cannot remove node '" + _Name + "': it still has children.";
    return false;
  }

  std::erase(GetSiblings(Node.GetParentName()), _Name);
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

  auto & Node = GetMutableNode(_Name);
  std::erase(GetSiblings(Node.m_ParentName), _Name);

  auto & NewSiblings = GetSiblings(_NewParentName);
  NewSiblings.insert(NewSiblings.begin() + static_cast<std::ptrdiff_t>(std::min(_Index, NewSiblings.size())), _Name);
  Node.m_ParentName = _NewParentName;

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
  NodeHandle.key()            = _NewName;
  NodeHandle.mapped()->m_Name = _NewName;

  auto & Node = *m_Nodes.insert(std::move(NodeHandle)).position->second;

  auto & Siblings = GetSiblings(Node.m_ParentName);
  std::replace(Siblings.begin(), Siblings.end(), _Name, _NewName);

  for (const auto & ChildName : Node.m_Children)
  {
    GetMutableNode(ChildName).m_ParentName = _NewName;
    MarkDirty(ChildName);
  }

  MarkDirty(_Name);
  MarkDirty(_NewName);

  return true;
}

bool CSceneGraph::ApplySnapshot(const std::string & _Name, const TSceneNodeSnapshot & _Snapshot, std::string & _Error)
{
  const auto NodeIterator = m_Nodes.find(_Name);
  if (NodeIterator == m_Nodes.end())
  {
    _Error = "Cannot change node '" + _Name + "': it does not exist.";
    return false;
  }

  auto Node = CreateNode(_Snapshot, _Error);
  if (!Node)
    return false;

  auto & OldNode = *NodeIterator->second;
  Node->m_Name       = std::move(OldNode.m_Name);
  Node->m_ParentName = std::move(OldNode.m_ParentName);
  Node->m_Children   = std::move(OldNode.m_Children);

  NodeIterator->second = std::move(Node);

  MarkDirty(_Name);

  return true;
}

bool CSceneGraph::EditNode(const std::string & _Name, const std::function<bool(CSceneNode &)> & _Edit)
{
  if (!HasNode(_Name) || !_Edit(GetMutableNode(_Name)))
    return false;

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

std::unique_ptr<CSceneNode> CSceneGraph::CreateNode(const TSceneNodeSnapshot & _Snapshot, std::string & _Error) const
{
  auto Node = m_NodeTypes->Create(_Snapshot.TypeId);
  if (!Node)
  {
    _Error = "Unknown node type '" + _Snapshot.TypeId + "'.";
    return nullptr;
  }

  if (!Node->ApplySnapshot(_Snapshot, _Error))
    return nullptr;

  return Node;
}

CSceneNode & CSceneGraph::GetMutableNode(const std::string & _Name)
{
  const auto NodeIterator = m_Nodes.find(_Name);
  assert(NodeIterator != m_Nodes.end() && "Scene node does not exist");

  return *NodeIterator->second;
}

std::vector<std::string> & CSceneGraph::GetSiblings(const std::string & _ParentName)
{
  if (_ParentName.empty())
    return m_RootNodes;

  const auto ParentIterator = m_Nodes.find(_ParentName);
  assert(ParentIterator != m_Nodes.end() && "Scene node parent does not exist");

  return ParentIterator->second->m_Children;
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
  _Subtree.Nodes.push_back({Node.GetName(), Node.GetParentName(), Node.CaptureSnapshot()});

  for (const auto & ChildName : Node.GetChildren())
    CaptureNode(ChildName, _Subtree);
}

} // namespace editor
