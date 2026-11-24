#pragma once

#include "SceneNodeBinding.h"
#include "modules/scene/model/nodes/MeshNode.h"

#include <OgrePrerequisites.h>

#include <string>

namespace editor
{

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

  void DestroyEntity();

private: // Attributes

  Ogre::SceneManager * m_SceneManager;
  Ogre::SceneNode *    m_Node;
  CSceneMeshCache &    m_Meshes;
  Ogre::Entity *       m_Entity;
  std::string          m_MeshName;
};

} // namespace editor
