#pragma once

#include "SceneNodeBinding.h"
#include "modules/scene/model/nodes/MeshNode.h"

#include <OgrePrerequisites.h>

#include <string>

namespace editor
{

class CAssetDatabase;
class CSceneMeshCache;
struct TSceneBindingContext;

class CMeshNodeBinding final:
  public CSceneNodeBinding<CMeshNode>
{
public: // Constructor/Destructor

  explicit CMeshNodeBinding(const TSceneBindingContext & _Context);

  ~CMeshNodeBinding() override;

protected: // CSceneNodeBinding

  bool OnApply(const CMeshNode & _Node, std::string & _Error) override;

private: // Implementation

  bool CreateEntity(const CMeshNode & _Node, std::string & _Error);

  void AttachEntity(Ogre::Entity * _Entity);

  void DestroyEntity();

private: // Attributes

  Ogre::SceneManager *   m_SceneManager;
  Ogre::SceneNode *      m_Node;
  CSceneMeshCache &      m_Meshes;
  const CAssetDatabase & m_Assets;
  Ogre::Entity *         m_Entity;
  std::string            m_MeshPath;
  bool                   m_IsMissing;
};

} // namespace editor
