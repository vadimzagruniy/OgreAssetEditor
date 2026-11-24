#pragma once

#include "SceneNodeBinding.h"
#include "modules/scene/model/nodes/PrimitiveNode.h"

#include <OgrePrerequisites.h>

#include <optional>
#include <string>

namespace editor
{

struct TSceneBindingContext;

class CPrimitiveNodeBinding final:
  public CSceneNodeBinding<CPrimitiveNode>
{
public: // Constructor/Destructor

  explicit CPrimitiveNodeBinding(const TSceneBindingContext & _Context);

  ~CPrimitiveNodeBinding() override;

protected: // CSceneNodeBinding

  bool OnApply(const CPrimitiveNode & _Node, std::string & _Error) override;

private: // Implementation

  void DestroyEntity();

private: // Attributes

  Ogre::SceneManager *           m_SceneManager;
  Ogre::SceneNode *              m_Node;
  Ogre::Entity *                 m_Entity;
  std::optional<EPrimitiveShape> m_Shape;
};

} // namespace editor
