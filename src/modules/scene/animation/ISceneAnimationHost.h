#pragma once

#include <OgrePrerequisites.h>

#include <string>

namespace editor
{

class CSceneGraph;

class ISceneAnimationHost
{
public: // Destructor

  virtual ~ISceneAnimationHost();

public: // Interface

  virtual const CSceneGraph & GetGraph() const = 0;

  virtual Ogre::SceneNode * FindSceneNode(const std::string & _Name) const = 0;

  virtual void RestoreNode(const std::string & _Name) = 0;
};

} // namespace editor
