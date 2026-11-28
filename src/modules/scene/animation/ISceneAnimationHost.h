#pragma once

#include "editor/inspector/PropertyValue.h"

#include <string>
#include <vector>

namespace editor
{

class CSceneGraph;
struct TAssetId;
struct TAssetReference;

class ISceneAnimationHost
{
public: // Destructor

  virtual ~ISceneAnimationHost();

public: // Interface

  virtual const CSceneGraph & GetGraph() const = 0;

  virtual std::string GetSelectedNode() const = 0;

  virtual void SelectNode(const std::string & _Name) = 0;

  virtual void PreviewValue(const std::string & _Name, const std::string & _Identifier, const TPropertyValue & _Value) = 0;

  virtual void RestoreNode(const std::string & _Name) = 0;

  virtual std::vector<TAssetReference> GetAnimationRefs() const = 0;

  virtual bool HasAnimation(const TAssetId & _AnimationId) const = 0;

  virtual void AddAnimation(const TAssetReference & _AnimationRef) = 0;

  virtual void RemoveAnimation(const TAssetReference & _AnimationRef) = 0;

  virtual std::string ResolveSlot(const TAssetId & _AnimationId, const std::string & _Slot) const = 0;

  virtual std::string FindSlot(const TAssetId & _AnimationId, const std::string & _NodeName) const = 0;

  virtual void BindSlot(const TAssetId & _AnimationId, const std::string & _Slot, const std::string & _NodeName) = 0;
};

} // namespace editor
