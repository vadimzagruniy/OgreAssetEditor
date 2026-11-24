#include "SceneSelection.h"

#include <utility>

namespace editor
{

//
// Interface
//

const std::string & CSceneSelection::GetNode() const noexcept
{
  return m_Node;
}

void CSceneSelection::Select(std::string _Name)
{
  m_Node = std::move(_Name);
}

void CSceneSelection::Clear() noexcept
{
  m_Node.clear();
}

} // namespace editor
