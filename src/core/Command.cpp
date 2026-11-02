#include "Command.h"

namespace editor::core
{

//
// Destructor
//

ICommand::~ICommand() = default;

//
// Interface
//

bool ICommand::MergeWith(const ICommand & /*_Next*/)
{
  return false;
}

} // namespace editor::core
