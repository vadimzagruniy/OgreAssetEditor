#pragma once

#include <string>

namespace editor
{

class CSceneDocument;
struct TSceneDrop;

class ISceneDropHandler
{
public: // Destructor

  virtual ~ISceneDropHandler();

public: // Interface

  virtual bool CanDrop(const TSceneDrop & _Drop, const CSceneDocument & _Document) const = 0;

  virtual std::string GetDropHint(const TSceneDrop & _Drop) const = 0;

  virtual void Drop(const TSceneDrop & _Drop, CSceneDocument & _Document) const = 0;
};

} // namespace editor
