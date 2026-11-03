#include "SceneEditorController.h"
#include "core/ActionQueue.h"
#include "modules/scene/SceneDocument.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CSceneEditorController::CSceneEditorController(std::shared_ptr<core::CActionQueue> _ActionQueue):
  m_ActionQueue(std::move(_ActionQueue))
{
  // Empty
}

//
// ISceneEditorPresenter state
//

std::shared_ptr<const CSceneDocument> CSceneEditorController::GetDocument() const
{
  return m_Document.lock();
}

//
// ISceneEditorPresenter actions
//

void CSceneEditorController::Select(const std::string & _Name)
{
  if (const auto Document = m_Document.lock())
    Document->Select(_Name);
}

void CSceneEditorController::CreateNode(const std::string & _ParentName)
{
  Post([_ParentName](CSceneDocument & _Document)
  {
    _Document.CreateNode(_ParentName);
  });
}

void CSceneEditorController::DeleteNode(const std::string & _Name)
{
  Post([_Name](CSceneDocument & _Document)
  {
    _Document.DeleteNode(_Name);
  });
}

void CSceneEditorController::MoveNode(const std::string & _Name, const std::string & _NewParentName)
{
  Post([_Name, _NewParentName](CSceneDocument & _Document)
  {
    _Document.MoveNode(_Name, _NewParentName);
  });
}

void CSceneEditorController::RenameNode(const std::string & _Name, const std::string & _NewName)
{
  Post([_Name, _NewName](CSceneDocument & _Document)
  {
    _Document.RenameNode(_Name, _NewName);
  });
}

//
// Interface
//

void CSceneEditorController::SetDocument(const std::shared_ptr<CSceneDocument> & _Document)
{
  m_Document = _Document;
}

void CSceneEditorController::ResetDocument(const CSceneDocument & _Document)
{
  if (m_Document.lock().get() == &_Document)
    m_Document.reset();
}

//
// Implementation
//

void CSceneEditorController::Post(std::function<void(CSceneDocument &)> _Action)
{
  m_ActionQueue->Post([Document = m_Document, Action = std::move(_Action)]
  {
    if (const auto LockedDocument = Document.lock())
      Action(*LockedDocument);
  });
}

} // namespace editor
