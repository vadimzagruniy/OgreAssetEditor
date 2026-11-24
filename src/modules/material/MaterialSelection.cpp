#include "MaterialSelection.h"

namespace editor
{

//
// Interface
//

const TMaterialNode & CMaterialSelection::GetNode() const noexcept
{
  return m_Node;
}

void CMaterialSelection::Select(const TMaterialNode & _Node) noexcept
{
  m_Node = _Node;
}

} // namespace editor
