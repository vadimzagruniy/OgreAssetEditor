#pragma once

#include "OgreNodeBinding.h"

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
class CSceneInstance;
struct TSceneNode;

class CSceneSync
{
public: // Constructor/Destructor

  CSceneSync(CRenderScene & _RenderScene, std::shared_ptr<core::CLog> _Log);

  ~CSceneSync();

  CSceneSync(const CSceneSync &) = delete;

  CSceneSync & operator=(const CSceneSync &) = delete;

public: // Interface

  bool Update(CSceneInstance & _Scene);

  void SetHighlightedNode(const std::string & _Name);

  std::string Pick(const Ogre::Ray & _Ray) const;

  std::optional<float> PickDistance(const Ogre::Ray & _Ray) const;

private: // Implementation

  bool FindHit(const Ogre::Ray & _Ray, std::string & _Name, float & _Distance) const;

  void ShowBounds(const std::string & _Name, bool _IsVisible);

  void DestroyRemovedNodes(const CSceneInstance & _Scene, const std::unordered_set<std::string> & _DirtyNodes);

  void CreateAddedNodes(const CSceneInstance & _Scene, const std::unordered_set<std::string> & _DirtyNodes);

  void ApplyNode(CSceneInstance & _Scene, const std::string & _Name);

  void ApplyParent(TOgreNodeBinding & _Binding, const TSceneNode & _Node);

  void ApplyMesh(CSceneInstance & _Scene, TOgreNodeBinding & _Binding, const TSceneNode & _Node);

  void DestroyEntity(TOgreNodeBinding & _Binding);

  void DestroyBinding(TOgreNodeBinding & _Binding);

private: // Attributes

  CRenderScene &                                    m_RenderScene;
  std::shared_ptr<core::CLog>                       m_Log;
  std::unordered_map<std::string, TOgreNodeBinding> m_Bindings;
  std::string                                       m_HighlightedNode;
};

} // namespace editor
