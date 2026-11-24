#pragma once

#include <OgrePrerequisites.h>

namespace editor
{

class CSceneMeshCache;

struct TSceneBindingContext
{
  Ogre::SceneManager * SceneManager;
  Ogre::SceneNode *    Node;
  CSceneMeshCache &    Meshes;
};

} // namespace editor
