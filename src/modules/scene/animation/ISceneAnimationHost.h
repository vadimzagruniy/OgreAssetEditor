#pragma once

#include "editor/inspector/PropertyValue.h"

#include <string>
#include <vector>

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

  virtual std::vector<std::string> GetAnimations() const = 0;

  virtual bool HasAnimation(const std::string & _Path) const = 0;

  virtual void AddAnimation(const std::string & _Path) = 0;

  virtual void RemoveAnimation(const std::string & _Path) = 0;

  virtual std::string ResolveSlot(const std::string & _Path, const std::string & _Slot) const = 0;

  virtual std::string FindSlot(const std::string & _Path, const std::string & _NodeName) const = 0;

  virtual void BindSlot(const std::string & _Path, const std::string & _Slot, const std::string & _NodeName) = 0;
};

} // namespace editor
