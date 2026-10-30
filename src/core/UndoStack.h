#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace editor::core
{

class ICommand;

class CUndoStack
{
public: // Constructor/Destructor

  CUndoStack();

  ~CUndoStack();

public: // Interface

  void Execute(std::unique_ptr<ICommand> _Command);

  bool Undo();

  bool Redo();

  void Clear();

  bool CanUndo() const noexcept;

  bool CanRedo() const noexcept;

  std::string GetUndoName() const;

  std::string GetRedoName() const;

  void MarkSaved() noexcept;

  bool IsAtSavePoint() const noexcept;

private: // Attributes

  std::vector<std::unique_ptr<ICommand>> m_Commands;
  std::size_t                            m_Cursor;
  std::size_t                            m_SavedCursor;
};

} // namespace editor::core
