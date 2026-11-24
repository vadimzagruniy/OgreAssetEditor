#include "SceneClipboard.h"

#include <utility>

namespace editor
{

//
// Interface
//

bool CSceneClipboard::IsEmpty() const noexcept
{
  return !m_Subtree.has_value();
}

const std::optional<TSceneSubtree> & CSceneClipboard::GetSubtree() const noexcept
{
  return m_Subtree;
}

void CSceneClipboard::Store(TSceneSubtree _Subtree)
{
  m_Subtree = std::move(_Subtree);
}

} // namespace editor
