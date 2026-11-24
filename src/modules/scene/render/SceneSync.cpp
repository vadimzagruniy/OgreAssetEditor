#include "SceneSync.h"
#include "core/Log.h"
#include "editor/assets/ResourceRelease.h"
#include "editor/render/RenderScene.h"
#include "modules/scene/SceneDocument.h"
#include "modules/scene/model/SceneGraph.h"

#include <OgreEntity.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreRay.h>
#include <OgreResourceGroupManager.h>
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

CSceneSync::CSceneSync(CRenderScene & _RenderScene, std::string _ResourceGroup, std::shared_ptr<core::CLog> _Log):
  m_RenderScene(_RenderScene),
  m_ResourceGroup(std::move(_ResourceGroup)),
  m_Log(std::move(_Log))
{
  // Empty
}

CSceneSync::~CSceneSync()
{
  for (auto & Entry : m_Bindings)
    DestroyBinding(Entry.second);

  for (auto & Entry : m_Meshes)
    ReleaseOgreResource(std::move(Entry.second));
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

    m_Bindings.emplace(Name, Binding);
  }
}

void CSceneSync::ApplyNode(const CSceneGraph & _Graph, const std::string & _Name)
{
  const auto & Node    = _Graph.GetNode(_Name);
  auto &       Binding = m_Bindings.find(_Name)->second;

  ApplyParent(Binding, Node);

  const auto & Transform = Node.Properties.Transform;
  Binding.Node->setPosition(Transform.Position);
  Binding.Node->setOrientation(Transform.Orientation);
  Binding.Node->setScale(Transform.Scale);

  ApplyMesh(Binding, Node);
}

void CSceneSync::ApplyParent(TOgreNodeBinding & _Binding, const TSceneNode & _Node)
{
  Ogre::SceneNode * Parent = m_RenderScene.GetContentRoot();
  if (!_Node.ParentName.empty())
    Parent = m_Bindings.find(_Node.ParentName)->second.Node;

  if (_Binding.Node->getParent() == Parent)
    return;

  if (auto * OldParent = _Binding.Node->getParent())
    OldParent->removeChild(_Binding.Node);

  Parent->addChild(_Binding.Node);
}

void CSceneSync::ApplyMesh(TOgreNodeBinding & _Binding, const TSceneNode & _Node)
{
  const auto & MeshName = _Node.Properties.MeshName;
  if (_Binding.MeshName == MeshName)
    return;

  DestroyEntity(_Binding);
  _Binding.MeshName = MeshName;

  if (!MeshName)
    return;

  Ogre::MeshPtr Mesh;
  std::string   Error;
  if (!AcquireMesh(*MeshName, Mesh, Error))
  {
    if (m_Log)
      m_Log->Write(core::ELogLevel::Warning, "Scene node '" + _Node.Name + "': " + Error);

    return;
  }

  _Binding.Entity = m_RenderScene.GetSceneManager()->createEntity(Mesh);
  _Binding.Entity->getUserObjectBindings().setUserAny(NodeNameBinding, Ogre::Any(_Node.Name));
  _Binding.Node->attachObject(_Binding.Entity);
}

bool CSceneSync::AcquireMesh(const std::string & _MeshName, Ogre::MeshPtr & _Mesh, std::string & _Error)
{
  if (const auto MeshIterator = m_Meshes.find(_MeshName); MeshIterator != m_Meshes.end())
  {
    _Mesh = MeshIterator->second;
    return true;
  }

  if (!Ogre::ResourceGroupManager::getSingleton().resourceExists(m_ResourceGroup, _MeshName))
  {
    _Error = "Mesh '" + _MeshName + "' is not found in the project resources.";
    return false;
  }

  _Mesh = Ogre::MeshManager::getSingleton().load(_MeshName, m_ResourceGroup);
  if (!_Mesh)
  {
    _Error = "OGRE returned an empty mesh: " + _MeshName;
    return false;
  }

  m_Meshes.emplace(_MeshName, _Mesh);

  return true;
}

void CSceneSync::DestroyEntity(TOgreNodeBinding & _Binding)
{
  if (!_Binding.Entity)
    return;

  _Binding.Node->detachObject(_Binding.Entity);
  m_RenderScene.GetSceneManager()->destroyEntity(_Binding.Entity);
  _Binding.Entity = nullptr;
}

void CSceneSync::DestroyBinding(TOgreNodeBinding & _Binding)
{
  DestroyEntity(_Binding);

  _Binding.Node->removeAllChildren();
  m_RenderScene.GetSceneManager()->destroySceneNode(_Binding.Node);
  _Binding.Node = nullptr;
}

} // namespace editor
