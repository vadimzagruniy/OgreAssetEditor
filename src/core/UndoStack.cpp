#include "UndoStack.h"
#include "Command.h"

#include <limits>

namespace editor::core
{

namespace
{

constexpr std::size_t UnreachableCursor = std::numeric_limits<std::size_t>::max();

} // namespace

//
// Constructor/Destructor
//

CUndoStack::CUndoStack():
  m_Cursor(0),
  m_SavedCursor(0)
{
  // Empty
}

CUndoStack::~CUndoStack() = default;

//
// Interface
//

void CUndoStack::Execute(std::unique_ptr<ICommand> _Command)
{
  if (!_Command)
    return;

  _Command->Execute();

  if (m_SavedCursor > m_Cursor)
    m_SavedCursor = UnreachableCursor;

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

  m_Cursor      = 0;
  m_SavedCursor = 0;
}

bool CUndoStack::CanUndo() const noexcept
{
  return m_Cursor > 0;
}

bool CUndoStack::CanRedo() const noexcept
{
  return m_Cursor < m_Commands.size();
}

std::string CUndoStack::GetUndoName() const
{
  return CanUndo() ? m_Commands[m_Cursor - 1]->GetName() : std::string();
}

std::string CUndoStack::GetRedoName() const
{
  return CanRedo() ? m_Commands[m_Cursor]->GetName() : std::string();
}

void CUndoStack::MarkSaved() noexcept
{
  m_SavedCursor = m_Cursor;
}

bool CUndoStack::IsAtSavePoint() const noexcept
{
  return m_Cursor == m_SavedCursor;
}

} // namespace editor::core
