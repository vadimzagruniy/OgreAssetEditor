#pragma once

#include "ISceneNodeBinding.h"

#include <string>

namespace editor
{

template <class TNode>
class CSceneNodeBinding:
  public ISceneNodeBinding
{
public: // ISceneNodeBinding

  bool Apply(const CSceneNode & _Node, std::string & _Error) final;

protected: // Events

  virtual bool OnApply(const TNode & _Node, std::string & _Error) = 0;
};

//
// Template implementation
//

template <class TNode>
bool CSceneNodeBinding<TNode>::Apply(const CSceneNode & _Node, std::string & _Error)
{
  return OnApply(static_cast<const TNode &>(_Node), _Error);
}

} // namespace editor
