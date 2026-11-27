#include "SceneDocument.h"
#include "SceneChange.h"
#include "SceneSerializer.h"
#include "model/SceneNode.h"
#include "core/FunctionCommand.h"
#include "core/WriteFileAtomic.h"
#include "editor/assets/AssetRecord.h"
#include "editor/documents/AssetProperty.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <unordered_map>
#include <utility>

namespace editor
{

namespace
{

void ExpectSuccess(bool _IsDone)
{
  assert(_IsDone && "Scene command failed on a consistent undo stack");
  (void)_IsDone;
}

std::vector<TSceneAnimation> RenameBindings(std::vector<TSceneAnimation> _Animations, const std::string & _Name, const std::string & _NewName)
{
  for (auto & Animation : _Animations)
  {
    for (auto & [Slot, NodeName] : Animation.Bindings)
    {
      if (NodeName == _Name)
        NodeName = _NewName;
    }

    Animation.Bindings.try_emplace(_Name, _NewName);

    std::erase_if(Animation.Bindings, [](const auto & _Binding)
    {
      return _Binding.first == _Binding.second;
    });
  }

  return _Animations;
}

bool IsFinite(const TSceneTransform & _Transform)
{
  const auto & Orientation = _Transform.Orientation;

  for (int Index = 0; Index < 3; ++Index)
  {
    if (!std::isfinite(_Transform.Position[Index]) || !std::isfinite(_Transform.Scale[Index]))
      return false;
  }

  return std::isfinite(Orientation.w) && std::isfinite(Orientation.x) && std::isfinite(Orientation.y) && std::isfinite(Orientation.z);
}

bool ToLocalTransform(const CSceneGraph & _Graph, const std::string & _ParentName, const Ogre::Affine3 & _World, TSceneTransform & _Transform)
{
  Ogre::Affine3 Local = _World;
  if (!_ParentName.empty())
    Local = _Graph.GetWorldTransform(_ParentName).inverse() * Local;

  TSceneTransform Transform;
  Local.decomposition(Transform.Position, Transform.Scale, Transform.Orientation);

  if (!IsFinite(Transform))
    return false;

  _Transform = Transform;

  return true;
}

} // namespace

//
// Constructor
//

CSceneDocument::CSceneDocument(TAssetRecord _Record, std::shared_ptr<const CSceneNodeTypes> _NodeTypes):
  CAssetDocument(std::move(_Record), "Scene"),
  m_Graph(std::move(_NodeTypes))
{
  // Empty
}

//
// CAssetDocument
//

std::vector<TAssetProperty> CSceneDocument::GetProperties() const
{
  return {{"Nodes", std::to_string(m_Graph.GetNodeCount())}};
}

bool CSceneDocument::OnSave(std::string & _Error)
{
  CommitPreview();

  return core::WriteFileAtomic(GetRecord().AbsolutePath, WriteScene(m_Graph, m_Animations), _Error);
}

//
// Interface
//

bool CSceneDocument::Load(const std::string & _Text, std::string & _Error)
{
  return ReadScene(_Text, m_Graph, m_Animations, _Error);
}

const CSceneGraph & CSceneDocument::GetGraph() const noexcept
{
  return m_Graph;
}

const std::vector<TSceneAnimation> & CSceneDocument::GetAnimations() const noexcept
{
  return m_Animations;
}

bool CSceneDocument::HasAnimation(const std::string & _Path) const
{
  return FindAnimation(_Path) != m_Animations.end();
}

std::string CSceneDocument::ResolveSlot(const std::string & _Path, const std::string & _Slot) const
{
  const auto Animation = FindAnimation(_Path);
  if (Animation == m_Animations.end())
    return _Slot;

  const auto Binding = Animation->Bindings.find(_Slot);
  return Binding == Animation->Bindings.end() ? _Slot : Binding->second;
}

std::string CSceneDocument::FindSlot(const std::string & _Path, const std::string & _NodeName) const
{
  const auto Animation = FindAnimation(_Path);
  if (Animation == m_Animations.end())
    return _NodeName;

  for (const auto & [Slot, NodeName] : Animation->Bindings)
  {
    if (NodeName == _NodeName)
      return Slot;
  }

  return _NodeName;
}

std::unordered_set<std::string> CSceneDocument::TakeDirtyNodes()
{
  return m_Graph.TakeDirtyNodes();
}

bool CSceneDocument::CanRename(const std::string & _Name, const std::string & _NewName, std::string & _Error) const
{
  if (_Name == _NewName)
    return true;

  if (_NewName.empty())
  {
    _Error = "Name cannot be empty.";
    return false;
  }

  if (m_Graph.HasNode(_NewName))
  {
    _Error = "Name '" + _NewName + "' is already used.";
    return false;
  }

  return true;
}

bool CSceneDocument::CanMove(const std::string & _Name, const std::string & _NewParentName) const
{
  if (!m_Graph.HasNode(_Name))
    return false;

  if (_NewParentName.empty())
    return true;

  return m_Graph.HasNode(_NewParentName) && !m_Graph.IsSameOrDescendant(_NewParentName, _Name);
}

void CSceneDocument::CreateNode(const std::string & _ParentName, const std::string & _BaseName, const TSceneNodeSnapshot & _Snapshot)
{
  if (!_ParentName.empty() && !m_Graph.HasNode(_ParentName))
    return;

  TSceneSubtree Subtree;
  Subtree.Nodes.push_back({_BaseName, _ParentName, _Snapshot});

  InsertSubtree("Create Node", Subtree, _ParentName, m_Graph.GetChildren(_ParentName).size());
}

void CSceneDocument::PasteSubtree(const TSceneSubtree & _Subtree, const std::string & _ParentName, std::size_t _Index)
{
  InsertSubtree("Paste Node", _Subtree, _ParentName, _Index);
}

void CSceneDocument::DuplicateNode(const std::string & _Name)
{
  if (!m_Graph.HasNode(_Name))
    return;

  InsertSubtree("Duplicate Node", m_Graph.CaptureSubtree(_Name), m_Graph.GetNode(_Name).GetParentName(), m_Graph.GetSiblingIndex(_Name) + 1);
}

void CSceneDocument::DeleteNode(const std::string & _Name)
{
  if (!m_Graph.HasNode(_Name))
    return;

  ExecuteCommand("Delete Node",
    [_Name](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Graph.RemoveSubtree(_Name, Error));
      _Document.Notify(ESceneChangeKind::Removed, _Name);
    },
    [Subtree = m_Graph.CaptureSubtree(_Name)](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Graph.InsertSubtree(Subtree, Error));
      _Document.Notify(ESceneChangeKind::Added, Subtree.Nodes.front().Name);
    });
}

void CSceneDocument::MoveNode(const std::string & _Name, const std::string & _NewParentName)
{
  if (!CanMove(_Name, _NewParentName))
    return;

  const auto &      Node          = m_Graph.GetNode(_Name);
  const std::string OldParentName = Node.GetParentName();
  const std::size_t OldIndex      = m_Graph.GetSiblingIndex(_Name);
  const auto        OldSnapshot   = Node.CaptureSnapshot();

  auto NewSnapshot = OldSnapshot;
  ToLocalTransform(m_Graph, _NewParentName, m_Graph.GetWorldTransform(_Name), NewSnapshot.Transform);

  const std::size_t NewIndex = m_Graph.GetChildren(_NewParentName).size() - (OldParentName == _NewParentName ? 1 : 0);
  if (OldParentName == _NewParentName && OldIndex == NewIndex)
    return;

  auto Move = [_Name](const std::string & _ParentName, std::size_t _Index, const TSceneNodeSnapshot & _Snapshot)
  {
    return [_Name, _ParentName, _Index, _Snapshot](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Graph.MoveNode(_Name, _ParentName, _Index, Error));
      ExpectSuccess(_Document.m_Graph.ApplySnapshot(_Name, _Snapshot, Error));
      _Document.Notify(ESceneChangeKind::Changed, _Name);
    };
  };

  ExecuteCommand("Move Node", Move(_NewParentName, NewIndex, NewSnapshot), Move(OldParentName, OldIndex, OldSnapshot));
}

void CSceneDocument::RenameNode(const std::string & _Name, const std::string & _NewName)
{
  std::string ValidationError;
  if (_Name == _NewName || !m_Graph.HasNode(_Name) || !CanRename(_Name, _NewName, ValidationError))
    return;

  auto Rename = [](const std::string & _From, const std::string & _To, std::vector<TSceneAnimation> _Animations)
  {
    return [_From, _To, Animations = std::move(_Animations)](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Graph.RenameNode(_From, _To, Error));
      _Document.m_Animations = Animations;
      _Document.Notify(ESceneChangeKind::Renamed, _From, _To);
    };
  };

  ExecuteCommand("Rename Node", Rename(_Name, _NewName, RenameBindings(m_Animations, _Name, _NewName)), Rename(_NewName, _Name, m_Animations));
}

void CSceneDocument::AddAnimation(const std::string & _Path)
{
  if (_Path.empty() || HasAnimation(_Path))
    return;

  auto Animations = m_Animations;
  Animations.push_back({_Path, {}});

  ReplaceAnimations("Add Animation", std::move(Animations));
}

void CSceneDocument::RemoveAnimation(const std::string & _Path)
{
  auto Animations = m_Animations;
  std::erase_if(Animations, [&_Path](const TSceneAnimation & _Animation)
  {
    return _Animation.Path == _Path;
  });

  ReplaceAnimations("Remove Animation", std::move(Animations));
}

void CSceneDocument::BindSlot(const std::string & _Path, const std::string & _Slot, const std::string & _NodeName)
{
  auto Animations = m_Animations;
  auto Animation  = std::find_if(Animations.begin(), Animations.end(), [&_Path](const TSceneAnimation & _Animation)
  {
    return _Animation.Path == _Path;
  });

  if (Animation == Animations.end() || _Slot.empty())
    return;

  if (_Slot == _NodeName)
    Animation->Bindings.erase(_Slot);
  else
    Animation->Bindings[_Slot] = _NodeName;

  ReplaceAnimations("Bind Slot", std::move(Animations));
}

void CSceneDocument::PreviewValue(const std::string & _Name, const std::string & _Identifier, const TPropertyValue & _Value, const std::string & _CommandName)
{
  if (!BeginPreview(_Name, _CommandName))
    return;

  m_Graph.EditNode(_Name, [&_Identifier, &_Value](CSceneNode & _Node)
  {
    return _Node.SetValue(_Identifier, _Value);
  });
}

void CSceneDocument::PreviewWorldTransform(const std::string & _Name, const Ogre::Affine3 & _World)
{
  if (!BeginPreview(_Name, "Transform Node"))
    return;

  TSceneTransform Transform;
  if (!ToLocalTransform(m_Graph, m_Graph.GetNode(_Name).GetParentName(), _World, Transform))
    return;

  m_Graph.EditNode(_Name, [&Transform](CSceneNode & _Node)
  {
    _Node.SetTransform(Transform);
    return true;
  });
}

void CSceneDocument::CommitPreview()
{
  if (!m_Preview)
    return;

  const TPreview Preview = std::move(*m_Preview);
  m_Preview.reset();

  if (!m_Graph.HasNode(Preview.Name))
    return;

  const auto After = m_Graph.GetNode(Preview.Name).CaptureSnapshot();
  if (After == Preview.Before)
    return;

  auto Apply = [Name = Preview.Name](const TSceneNodeSnapshot & _Snapshot)
  {
    return [Name, _Snapshot](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Graph.ApplySnapshot(Name, _Snapshot, Error));
      _Document.Notify(ESceneChangeKind::Changed, Name);
    };
  };

  ExecuteCommand(Preview.CommandName, Apply(After), Apply(Preview.Before));
}

void CSceneDocument::CancelPreview()
{
  if (!m_Preview)
    return;

  const TPreview Preview = std::move(*m_Preview);
  m_Preview.reset();

  std::string Error;
  if (m_Graph.HasNode(Preview.Name))
    ExpectSuccess(m_Graph.ApplySnapshot(Preview.Name, Preview.Before, Error));
}

//
// Signals
//

CSceneDocument::TChangedSignal & CSceneDocument::GetChangedSignal() noexcept
{
  return m_ChangedSignal;
}

//
// Implementation
//

void CSceneDocument::ExecuteCommand(std::string _CommandName, TEdit _Execute, TEdit _Undo)
{
  CommitPreview();

  auto Apply = [Document = weak_from_this()](const TEdit & _Edit)
  {
    if (const auto LockedDocument = Document.lock())
    {
      LockedDocument->m_Preview.reset();
      _Edit(*LockedDocument);
    }
  };

  GetHistory().Execute(std::make_unique<core::CFunctionCommand>(std::move(_CommandName),
    [Apply, Execute = std::move(_Execute)] { Apply(Execute); },
    [Apply, Undo = std::move(_Undo)] { Apply(Undo); }));
}

bool CSceneDocument::BeginPreview(const std::string & _Name, const std::string & _CommandName)
{
  if (!m_Graph.HasNode(_Name))
    return false;

  if (m_Preview && (m_Preview->Name != _Name || m_Preview->CommandName != _CommandName))
    CommitPreview();

  if (!m_Preview)
    m_Preview = TPreview{_Name, _CommandName, m_Graph.GetNode(_Name).CaptureSnapshot()};

  return true;
}

void CSceneDocument::InsertSubtree(std::string _CommandName, const TSceneSubtree & _Subtree, const std::string & _ParentName, std::size_t _Index)
{
  if (_Subtree.Nodes.empty() || (!_ParentName.empty() && !m_Graph.HasNode(_ParentName)))
    return;

  std::unordered_map<std::string, std::string> NewNames;
  std::unordered_set<std::string>              Reserved;

  TSceneSubtree Subtree;
  Subtree.Index = std::min(_Index, m_Graph.GetChildren(_ParentName).size());

  for (const auto & Node : _Subtree.Nodes)
  {
    const bool        IsRoot  = Subtree.Nodes.empty();
    const std::string NewName = m_Graph.MakeUniqueName(Node.Name, Reserved);

    Reserved.insert(NewName);
    NewNames.emplace(Node.Name, NewName);

    Subtree.Nodes.push_back({NewName, IsRoot ? _ParentName : NewNames.at(Node.ParentName), Node.Snapshot});
  }

  const std::string RootName = Subtree.Nodes.front().Name;

  ExecuteCommand(std::move(_CommandName),
    [Subtree](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Graph.InsertSubtree(Subtree, Error));
      _Document.Notify(ESceneChangeKind::Added, Subtree.Nodes.front().Name);
    },
    [RootName](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Graph.RemoveSubtree(RootName, Error));
      _Document.Notify(ESceneChangeKind::Removed, RootName);
    });
}

void CSceneDocument::Notify(ESceneChangeKind _Kind, const std::string & _Name, const std::string & _NewName) const
{
  m_ChangedSignal.Emit(TSceneChange{_Kind, _Name, _NewName});
}

void CSceneDocument::ReplaceAnimations(std::string _CommandName, std::vector<TSceneAnimation> _Animations)
{
  if (_Animations == m_Animations)
    return;

  auto Apply = [](std::vector<TSceneAnimation> _Value)
  {
    return [Value = std::move(_Value)](CSceneDocument & _Document)
    {
      _Document.m_Animations = Value;
      _Document.Notify(ESceneChangeKind::AnimationsChanged, {});
    };
  };

  ExecuteCommand(std::move(_CommandName), Apply(std::move(_Animations)), Apply(m_Animations));
}

std::vector<TSceneAnimation>::const_iterator CSceneDocument::FindAnimation(const std::string & _Path) const
{
  return std::find_if(m_Animations.begin(), m_Animations.end(), [&_Path](const TSceneAnimation & _Animation)
  {
    return _Animation.Path == _Path;
  });
}

} // namespace editor
