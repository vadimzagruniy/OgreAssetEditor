#pragma once

namespace editor
{

class CSceneGraph;

class ISceneAnimationHost
{
public: // Destructor

  virtual ~ISceneAnimationHost();

public: // Interface

  virtual const CSceneGraph & GetGraph() const = 0;
};

} // namespace editor
