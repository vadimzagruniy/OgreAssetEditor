#include "EditorService.h"

namespace editor::core
{

//
// Constructor
//

CEditorService::CEditorService(const CEditorService * _Parent) noexcept:
  m_Parent(_Parent)
{
  // Empty
}

} // namespace editor::core
