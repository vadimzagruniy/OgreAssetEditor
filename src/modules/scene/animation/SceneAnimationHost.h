#pragma once

#include "ISceneAnimationHost.h"

#include <memory>

namespace editor
{

class CSceneDocument;
class CSceneSync;

class CSceneAnimationHost final:
  public ISceneAnimationHost
{
public: // Constructor

  CSceneAnimationHost(std::shared_ptr<const CSceneDocument> _Document, CSceneSync & _Sync);

public: // ISceneAnimationHost

  const CSceneGraph & GetGraph() const override;

  Ogre::SceneNode * FindSceneNode(const std::string & _Name) const override;

  void RestoreNode(const std::string & _Name) override;

private: // Attributes

  std::shared_ptr<const CSceneDocument> m_Document;
  CSceneSync &                          m_Sync;
};

} // namespace editor
