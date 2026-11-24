#pragma once

#include <string>

namespace editor
{

class CSceneNode;

class ISceneNodeBinding
{
public: // Destructor

  virtual ~ISceneNodeBinding();

public: // Interface

  virtual bool Apply(const CSceneNode & _Node, std::string & _Error) = 0;
};

} // namespace editor
