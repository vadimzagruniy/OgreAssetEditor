#pragma once

#include "editor/inspector/PropertyValue.h"

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

  virtual std::string GetSelectedNode() const = 0;

  virtual void PreviewValue(const std::string & _Name, const std::string & _Identifier, const TPropertyValue & _Value) = 0;

  virtual void RestoreNode(const std::string & _Name) = 0;
};

} // namespace editor
