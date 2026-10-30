#include "MaterialEditingSession.h"
#include "MaterialDocument.h"

namespace editor
{

//
// Interface
//

void CMaterialEditingSession::SetDocument(const std::shared_ptr<CMaterialDocument> & _Document)
{
  m_Document = _Document;
}

void CMaterialEditingSession::ResetDocument(const CMaterialDocument & _Document)
{
  if (m_Document.lock().get() == &_Document)
    m_Document.reset();
}

std::shared_ptr<CMaterialDocument> CMaterialEditingSession::GetDocument() const
{
  return m_Document.lock();
}

} // namespace editor
