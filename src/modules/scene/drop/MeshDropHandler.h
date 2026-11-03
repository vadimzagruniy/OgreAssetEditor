#pragma once

#include "modules/scene/drop/ISceneDropHandler.h"

namespace editor
{

class CMeshDropHandler final:
  public ISceneDropHandler
{
public: // ISceneDropHandler

  bool CanDrop(const TSceneDrop & _Drop, const CSceneDocument & _Document) const override;

  std::string GetDropHint(const TSceneDrop & _Drop) const override;

  void Drop(const TSceneDrop & _Drop, CSceneDocument & _Document) const override;
};

} // namespace editor
