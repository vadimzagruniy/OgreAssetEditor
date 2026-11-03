#include "SceneDocument.h"
#include "SceneInstance.h"
#include "core/FunctionCommand.h"
#include "core/UndoStack.h"

#include <cassert>
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

TSceneNodeProperties WithWorldTransform(const CSceneInstance & _Instance, const std::string & _Name, const std::string & _ParentName, const Ogre::Affine3 & _World)
{
  auto Properties = _Instance.GetNode(_Name).Properties;

  Ogre::Affine3 Local = _World;
  if (!_ParentName.empty())
    Local = _Instance.GetWorldTransform(_ParentName).inverse() * Local;

  auto & Transform = Properties.Transform;
  Local.decomposition(Transform.Position, Transform.Scale, Transform.Orientation);

  return Properties;
}

} // namespace

//
// Constructor
//

CSceneDocument::CSceneDocument(CSceneInstance & _Instance, core::CUndoStack & _UndoStack):
  m_Instance(_Instance),
  m_UndoStack(_UndoStack)
{
  // Empty
}

//
// Interface
//

const CSceneInstance & CSceneDocument::GetInstance() const noexcept
{
  return m_Instance;
}

std::string CSceneDocument::GetSelection() const
{
  return m_Instance.HasNode(m_Selection) ? m_Selection : std::string();
}

void CSceneDocument::Select(const std::string & _Name)
{
  m_Selection = _Name;
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

  if (m_Instance.HasNode(_NewName))
  {
    _Error = "Name '" + _NewName + "' is already used.";
    return false;
  }

  return true;
}

bool CSceneDocument::CanMove(const std::string & _Name, const std::string & _NewParentName) const
{
  if (!m_Instance.HasNode(_Name))
    return false;

  if (_NewParentName.empty())
    return true;

  return m_Instance.HasNode(_NewParentName) && !m_Instance.IsSameOrDescendant(_NewParentName, _Name);
}

void CSceneDocument::CreateNode(const std::string & _ParentName)
{
  if (!_ParentName.empty() && !m_Instance.HasNode(_ParentName))
    return;

  const std::string Name = m_Instance.MakeUniqueName("Node");

  TSceneSubtree Subtree;
  Subtree.Index = m_Instance.GetChildren(_ParentName).size();
  Subtree.Nodes.push_back({Name, _ParentName, {}});

  ExecuteCommand("Create Node",
    [Subtree](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Instance.InsertSubtree(Subtree, Error));
      _Document.m_Selection = Subtree.Nodes.front().Name;
    },
    [Name](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Instance.RemoveSubtree(Name, Error));
    });
}

void CSceneDocument::DeleteNode(const std::string & _Name)
{
  if (!m_Instance.HasNode(_Name))
    return;

  ExecuteCommand("Delete Node",
    [_Name](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Instance.RemoveSubtree(_Name, Error));
    },
    [Subtree = m_Instance.CaptureSubtree(_Name)](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Instance.InsertSubtree(Subtree, Error));
      _Document.m_Selection = Subtree.Nodes.front().Name;
    });
}

void CSceneDocument::MoveNode(const std::string & _Name, const std::string & _NewParentName)
{
  if (!CanMove(_Name, _NewParentName))
    return;

  const auto &      Node          = m_Instance.GetNode(_Name);
  const std::string OldParentName = Node.ParentName;
  const std::size_t OldIndex      = m_Instance.GetSiblingIndex(_Name);
  const auto        OldProperties = Node.Properties;
  const auto        NewProperties = WithWorldTransform(m_Instance, _Name, _NewParentName, m_Instance.GetWorldTransform(_Name));

  const std::size_t NewIndex = m_Instance.GetChildren(_NewParentName).size() - (OldParentName == _NewParentName ? 1 : 0);
  if (OldParentName == _NewParentName && OldIndex == NewIndex)
    return;

  auto Move = [_Name](const std::string & _ParentName, std::size_t _Index, const TSceneNodeProperties & _Properties)
  {
    return [_Name, _ParentName, _Index, _Properties](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Instance.MoveNode(_Name, _ParentName, _Index, Error));
      ExpectSuccess(_Document.m_Instance.SetProperties(_Name, _Properties, Error));
    };
  };

  ExecuteCommand("Move Node", Move(_NewParentName, NewIndex, NewProperties), Move(OldParentName, OldIndex, OldProperties));
}

void CSceneDocument::RenameNode(const std::string & _Name, const std::string & _NewName)
{
  std::string ValidationError;
  if (_Name == _NewName || !m_Instance.HasNode(_Name) || !CanRename(_Name, _NewName, ValidationError))
    return;

  auto Rename = [](const std::string & _From, const std::string & _To)
  {
    return [_From, _To](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Instance.RenameNode(_From, _To, Error));

      if (_Document.m_Selection == _From)
        _Document.m_Selection = _To;
    };
  };

  ExecuteCommand("Rename Node", Rename(_Name, _NewName), Rename(_NewName, _Name));
}

void CSceneDocument::PreviewWorldTransform(const std::string & _Name, const Ogre::Affine3 & _World)
{
  if (!m_Instance.HasNode(_Name))
    return;

  if (m_Preview && m_Preview->Name != _Name)
    CommitPreview();

  const auto & Node = m_Instance.GetNode(_Name);
  if (!m_Preview)
    m_Preview = TPreview{_Name, Node.Properties};

  std::string Error;
  ExpectSuccess(m_Instance.SetProperties(_Name, WithWorldTransform(m_Instance, _Name, Node.ParentName, _World), Error));
}

void CSceneDocument::CommitPreview()
{
  if (!m_Preview)
    return;

  const TPreview Preview = std::move(*m_Preview);
  m_Preview.reset();

  if (!m_Instance.HasNode(Preview.Name))
    return;

  const auto After = m_Instance.GetNode(Preview.Name).Properties;
  if (After == Preview.Before)
    return;

  auto Apply = [Name = Preview.Name](const TSceneNodeProperties & _Properties)
  {
    return [Name, _Properties](CSceneDocument & _Document)
    {
      std::string Error;
      ExpectSuccess(_Document.m_Instance.SetProperties(Name, _Properties, Error));
      _Document.m_Selection = Name;
    };
  };

  ExecuteCommand("Transform Node", Apply(After), Apply(Preview.Before));
}

void CSceneDocument::CancelPreview()
{
  if (!m_Preview)
    return;

  const TPreview Preview = std::move(*m_Preview);
  m_Preview.reset();

  std::string Error;
  if (m_Instance.HasNode(Preview.Name))
    ExpectSuccess(m_Instance.SetProperties(Preview.Name, Preview.Before, Error));
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

  m_UndoStack.Execute(std::make_unique<core::CFunctionCommand>(std::move(_CommandName),
    [Apply, Execute = std::move(_Execute)] { Apply(Execute); },
    [Apply, Undo = std::move(_Undo)] { Apply(Undo); }));
}

} // namespace editor
