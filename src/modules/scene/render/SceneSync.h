#pragma once

#include "OgreNodeBinding.h"
#include "SceneMeshCache.h"

#include <OgrePrerequisites.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

class CRenderScene;
class CSceneDocument;
class CSceneGraph;
class CSceneNode;
class CSceneNodeBindings;

class CSceneSync
{
public: // Constructor/Destructor

  CSceneSync(CRenderScene & _RenderScene, std::string _ResourceGroup, std::shared_ptr<const CSceneNodeBindings> _NodeBindings, std::shared_ptr<core::CLog> _Log);

  ~CSceneSync();

  CSceneSync(const CSceneSync &) = delete;

  CSceneSync & operator=(const CSceneSync &) = delete;

public: // Interface

  bool Update(CSceneDocument & _Document);

  void SetHighlightedNode(const std::string & _Name);

  std::string Pick(const Ogre::Ray & _Ray) const;

  std::optional<float> PickDistance(const Ogre::Ray & _Ray) const;

  Ogre::SceneNode * FindOgreNode(const std::string & _Name) const;

  void RestoreTransform(const CSceneGraph & _Graph, const std::string & _Name);

private: // Implementation

  bool FindHit(const Ogre::Ray & _Ray, std::string & _Name, float & _Distance) const;

  void ShowBounds(const std::string & _Name, bool _IsVisible);

  void DestroyRemovedNodes(const CSceneGraph & _Graph, const std::unordered_set<std::string> & _DirtyNodes);

  void CreateAddedNodes(const CSceneGraph & _Graph, const std::unordered_set<std::string> & _DirtyNodes);

  void ApplyNode(const CSceneGraph & _Graph, const std::string & _Name);

  void ApplyParent(TOgreNodeBinding & _Binding, const CSceneNode & _Node);

  void ApplyObject(TOgreNodeBinding & _Binding, const CSceneNode & _Node);

  void DestroyBinding(TOgreNodeBinding & _Binding);

private: // Attributes

  CRenderScene &                                    m_RenderScene;
  std::shared_ptr<const CSceneNodeBindings>         m_NodeBindings;
  std::shared_ptr<core::CLog>                       m_Log;
  CSceneMeshCache                                   m_MeshCache;
  std::unordered_map<std::string, TOgreNodeBinding> m_Bindings;
  std::string                                       m_HighlightedNode;
};

} // namespace editor
