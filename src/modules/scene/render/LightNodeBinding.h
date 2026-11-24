#pragma once

#include "SceneNodeBinding.h"
#include "modules/scene/model/nodes/LightNode.h"

#include <OgrePrerequisites.h>

#include <string>

namespace editor
{

struct TSceneBindingContext;

class CLightNodeBinding final:
  public CSceneNodeBinding<CLightNode>
{
public: // Constructor/Destructor

  explicit CLightNodeBinding(const TSceneBindingContext & _Context);

  ~CLightNodeBinding() override;

protected: // CSceneNodeBinding

  bool OnApply(const CLightNode & _Node, std::string & _Error) override;

private: // Attributes

  Ogre::SceneManager * m_SceneManager;
  Ogre::SceneNode *    m_Node;
  Ogre::Light *        m_Light;
};

} // namespace editor
