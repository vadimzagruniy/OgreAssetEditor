#include "FunctionCommand.h"

#include <utility>

namespace editor::core
{

//
// Constructor
//

CFunctionCommand::CFunctionCommand(std::string _Name, std::function<void()> _Execute, std::function<void()> _Undo):
  m_Name(std::move(_Name)),
  m_Execute(std::move(_Execute)),
  m_Undo(std::move(_Undo))
{
  // Empty
}

//
// ICommand
//

std::string CFunctionCommand::GetName() const
{
  return m_Name;
}

void CFunctionCommand::Execute()
{
  m_Execute();
}

void CFunctionCommand::Undo()
{
  m_Undo();
}

} // namespace editor::core
