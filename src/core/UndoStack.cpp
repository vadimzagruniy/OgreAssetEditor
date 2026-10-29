#include "UndoStack.h"
#include "Command.h"

namespace editor::core
{

//
// Constructor/Destructor
//

CUndoStack::CUndoStack()
{
  // Empty
}

CUndoStack::~CUndoStack() = default;

//
// State
//

bool CUndoStack::CanUndo() const noexcept
{
  return m_Cursor > 0;
}

bool CUndoStack::CanRedo() const noexcept
{
  return m_Cursor < m_Commands.size();
}

//
// Interface
//

void CUndoStack::Execute(std::unique_ptr<ICommand> _Command)
{
  if (!_Command)
    return;

  _Command->Execute();

  m_Commands.erase(m_Commands.begin() + static_cast<std::ptrdiff_t>(m_Cursor), m_Commands.end());
  m_Commands.push_back(std::move(_Command));

  m_Cursor = m_Commands.size();
}

bool CUndoStack::Undo()
{
  if (!CanUndo())
    return false;

  m_Commands[--m_Cursor]->Undo();

  return true;
}

bool CUndoStack::Redo()
{
  if (!CanRedo())
    return false;

  m_Commands[m_Cursor++]->Execute();

  return true;
}

void CUndoStack::Clear()
{
  m_Commands.clear();
  m_Cursor = 0;
}

} // namespace editor::core
