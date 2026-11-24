#pragma once

#include "model/SceneSubtree.h"

#include <optional>

namespace editor
{

class CSceneClipboard final
{
public: // Interface

  bool IsEmpty() const noexcept;

  const std::optional<TSceneSubtree> & GetSubtree() const noexcept;

  void Store(TSceneSubtree _Subtree);

private: // Attributes

  std::optional<TSceneSubtree> m_Subtree;
};

} // namespace editor
