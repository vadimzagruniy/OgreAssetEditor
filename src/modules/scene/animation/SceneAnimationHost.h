#pragma once

#include "ISceneAnimationHost.h"

#include <memory>

namespace editor
{

class CSceneDocument;

class CSceneAnimationHost final:
  public ISceneAnimationHost
{
public: // Constructor

  explicit CSceneAnimationHost(std::shared_ptr<const CSceneDocument> _Document);

public: // ISceneAnimationHost

  const CSceneGraph & GetGraph() const override;

private: // Attributes

  std::shared_ptr<const CSceneDocument> m_Document;
};

} // namespace editor
