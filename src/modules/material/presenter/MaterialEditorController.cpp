#include "MaterialEditorController.h"
#include "core/ActionQueue.h"
#include "modules/material/MaterialDocument.h"
#include "modules/material/MaterialSelection.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CMaterialEditorController::CMaterialEditorController(std::shared_ptr<core::CActionQueue> _ActionQueue):
  m_ActionQueue(std::move(_ActionQueue))
{
  // Empty
}

//
// IMaterialEditorPresenter state
//

std::shared_ptr<const CMaterialDocument> CMaterialEditorController::GetDocument() const
{
  return m_Document.lock();
}

TMaterialNode CMaterialEditorController::GetSelection() const
{
  const auto Selection = m_Selection.lock();
  return Selection ? Selection->GetNode() : TMaterialNode{};
}

//
// IMaterialEditorPresenter actions
//

void CMaterialEditorController::Select(const TMaterialNode & _Node)
{
  Post([Selection = m_Selection, _Node](CMaterialDocument & _Document)
  {
    _Document.CommitPreview();

    if (const auto LockedSelection = Selection.lock())
      LockedSelection->Select(_Node);
  });
}

void CMaterialEditorController::Preview(const TMaterialNode & _Node, const TPassState & _State)
{
  Post([_Node, _State](CMaterialDocument & _Document)
  {
    _Document.Preview(_Node, _State);
  });
}

void CMaterialEditorController::Preview(const TMaterialNode & _Node, const TTextureUnitState & _State)
{
  Post([_Node, _State](CMaterialDocument & _Document)
  {
    _Document.Preview(_Node, _State);
  });
}

void CMaterialEditorController::CommitEdit()
{
  Post([](CMaterialDocument & _Document)
  {
    _Document.CommitPreview();
  });
}

//
// Interface
//

void CMaterialEditorController::SetDocument(const std::shared_ptr<CMaterialDocument> & _Document, const std::shared_ptr<CMaterialSelection> & _Selection)
{
  m_Document  = _Document;
  m_Selection = _Selection;
}

void CMaterialEditorController::ResetDocument(const CMaterialDocument & _Document)
{
  if (m_Document.lock().get() != &_Document)
    return;

  m_Document.reset();
  m_Selection.reset();
}

//
// Implementation
//

void CMaterialEditorController::Post(std::function<void(CMaterialDocument &)> _Action)
{
  m_ActionQueue->Post([Document = m_Document, Action = std::move(_Action)]
  {
    if (const auto LockedDocument = Document.lock())
      Action(*LockedDocument);
  });
}

} // namespace editor
