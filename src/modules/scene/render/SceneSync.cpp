#include "SceneSync.h"
#include "SceneBindingContext.h"
#include "SceneNodeBindings.h"
#include "core/Log.h"
#include "editor/render/RenderScene.h"
#include "modules/scene/SceneDocument.h"
#include "modules/scene/model/SceneGraph.h"
#include "modules/scene/model/SceneNode.h"

#include <OgreMovableObject.h>
#include <OgreRay.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreSceneQuery.h>

#include <utility>

namespace editor
{

namespace
{

const Ogre::String NodeNameBinding = "SceneNodeName";

} // namespace

//
// Constructor/Destructor
//

CSceneSync::CSceneSync(CRenderScene & _RenderScene, std::string _ResourceGroup, std::shared_ptr<const CSceneNodeBindings> _NodeBindings, std::shared_ptr<core::CLog> _Log):
  m_RenderScene(_RenderScene),
  m_NodeBindings(std::move(_NodeBindings)),
  m_Log(std::move(_Log)),
  m_MeshCache(std::move(_ResourceGroup))
{
  // Empty
}

CSceneSync::~CSceneSync()
{
  for (auto & Entry : m_Bindings)
    DestroyBinding(Entry.second);
}

//
// Interface
//

bool CSceneSync::Update(CSceneDocument & _Document)
{
  const auto DirtyNodes = _Document.TakeDirtyNodes();
  if (DirtyNodes.empty())
    return false;

  const auto & Graph = _Document.GetGraph();

  DestroyRemovedNodes(Graph, DirtyNodes);
  CreateAddedNodes(Graph, DirtyNodes);

  for (const auto & Name : DirtyNodes)
  {
    if (Graph.HasNode(Name))
      ApplyNode(Graph, Name);
  }

  return true;
}

void CSceneSync::SetHighlightedNode(const std::string & _Name)
{
  if (_Name != m_HighlightedNode)
  {
    ShowBounds(m_HighlightedNode, false);
    m_HighlightedNode = _Name;
  }

  ShowBounds(m_HighlightedNode, true);
}

std::string CSceneSync::Pick(const Ogre::Ray & _Ray) const
{
  std::string PickedName;
  float       Distance = 0.0f;

  FindHit(_Ray, PickedName, Distance);

  return PickedName;
}

std::optional<float> CSceneSync::PickDistance(const Ogre::Ray & _Ray) const
{
  std::string PickedName;
  float       Distance = 0.0f;

  if (!FindHit(_Ray, PickedName, Distance))
    return std::nullopt;

  return Distance;
}

void CSceneSync::ApplyPreview(const std::string & _Name, const CSceneNode & _Node)
{
  const auto BindingIterator = m_Bindings.find(_Name);
  if (BindingIterator == m_Bindings.end())
    return;

  auto &       Binding   = BindingIterator->second;
  const auto & Transform = _Node.GetTransform();

  Binding.Node->setPosition(Transform.Position);
  Binding.Node->setOrientation(Transform.Orientation);
  Binding.Node->setScale(Transform.Scale);

  ApplyObject(Binding, _Node);
}

void CSceneSync::RestoreNode(const CSceneGraph & _Graph, const std::string & _Name)
{
  if (_Graph.HasNode(_Name) && m_Bindings.contains(_Name))
    ApplyNode(_Graph, _Name);
}

//
// Implementation
//

bool CSceneSync::FindHit(const Ogre::Ray & _Ray, std::string & _Name, float & _Distance) const
{
  auto * SceneManager = m_RenderScene.GetSceneManager();
  auto * Query        = SceneManager->createRayQuery(_Ray);
  Query->setSortByDistance(true);

  bool IsHit = false;

  for (const auto & Entry : Query->execute())
  {
    if (!Entry.movable)
      continue;

    const auto & Binding = Entry.movable->getUserObjectBindings().getUserAny(NodeNameBinding);
    if (const auto * Name = Ogre::any_cast<std::string>(&Binding))
    {
      _Name     = *Name;
      _Distance = Entry.distance;
      IsHit     = true;
      break;
    }
  }

  SceneManager->destroyQuery(Query);

  return IsHit;
}

void CSceneSync::ShowBounds(const std::string & _Name, bool _IsVisible)
{
  const auto BindingIterator = m_Bindings.find(_Name);
  if (BindingIterator != m_Bindings.end())
    BindingIterator->second.Node->showBoundingBox(_IsVisible);
}

void CSceneSync::DestroyRemovedNodes(const CSceneGraph & _Graph, const std::unordered_set<std::string> & _DirtyNodes)
{
  for (const auto & Name : _DirtyNodes)
  {
    if (_Graph.HasNode(Name))
      continue;

    const auto BindingIterator = m_Bindings.find(Name);
    if (BindingIterator == m_Bindings.end())
      continue;

    DestroyBinding(BindingIterator->second);
    m_Bindings.erase(BindingIterator);
  }
}

void CSceneSync::CreateAddedNodes(const CSceneGraph & _Graph, const std::unordered_set<std::string> & _DirtyNodes)
{
  for (const auto & Name : _DirtyNodes)
  {
    if (!_Graph.HasNode(Name) || m_Bindings.contains(Name))
      continue;

    TOgreNodeBinding Binding;
    Binding.Node = m_RenderScene.GetContentRoot()->createChildSceneNode();

    m_Bindings.emplace(Name, std::move(Binding));
  }
}

void CSceneSync::ApplyNode(const CSceneGraph & _Graph, const std::string & _Name)
{
  const auto & Node    = _Graph.GetNode(_Name);
  auto &       Binding = m_Bindings.find(_Name)->second;

  ApplyParent(Binding, Node);

  const auto & Transform = Node.GetTransform();
  Binding.Node->setPosition(Transform.Position);
  Binding.Node->setOrientation(Transform.Orientation);
  Binding.Node->setScale(Transform.Scale);

  ApplyObject(Binding, Node);

  for (auto * Object : Binding.Node->getAttachedObjects())
    Object->getUserObjectBindings().setUserAny(NodeNameBinding, Ogre::Any(Node.GetName()));
}

void CSceneSync::ApplyParent(TOgreNodeBinding & _Binding, const CSceneNode & _Node)
{
  Ogre::SceneNode * Parent = m_RenderScene.GetContentRoot();
  if (!_Node.GetParentName().empty())
    Parent = m_Bindings.find(_Node.GetParentName())->second.Node;

  if (_Binding.Node->getParent() == Parent)
    return;

  if (auto * OldParent = _Binding.Node->getParent())
    OldParent->removeChild(_Binding.Node);

  Parent->addChild(_Binding.Node);
}

void CSceneSync::ApplyObject(TOgreNodeBinding & _Binding, const CSceneNode & _Node)
{
  const std::string TypeId(_Node.GetTypeId());
  if (_Binding.TypeId != TypeId)
  {
    _Binding.Object.reset();
    _Binding.TypeId = TypeId;
    _Binding.Object = m_NodeBindings->Create(TypeId, TSceneBindingContext{m_RenderScene.GetSceneManager(), _Binding.Node, m_MeshCache});
  }

  if (!_Binding.Object)
    return;

  std::string Error;
  if (!_Binding.Object->Apply(_Node, Error) && m_Log)
    m_Log->Write(core::ELogLevel::Warning, "Scene node '" + _Node.GetName() + "': " + Error);
}

void CSceneSync::DestroyBinding(TOgreNodeBinding & _Binding)
{
  _Binding.Object.reset();

  _Binding.Node->removeAllChildren();
  m_RenderScene.GetSceneManager()->destroySceneNode(_Binding.Node);
  _Binding.Node = nullptr;
}

} // namespace editor
