#pragma once

#include <OgrePrerequisites.h>

namespace editor
{

class CAssetDatabase;
class CSceneMeshCache;

struct TSceneBindingContext
{
  Ogre::SceneManager *   SceneManager;
  Ogre::SceneNode *      Node;
  CSceneMeshCache &      Meshes;
  const CAssetDatabase & Assets;
};

} // namespace editor
