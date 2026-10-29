#include "IModule.h"

namespace editor
{

//
// Destructor
//

IModule::~IModule() = default;

//
// Interface
//

void IModule::Shutdown() noexcept
{
  // Empty
}

void IModule::Tick(float /*_DeltaTime*/)
{
  // Empty
}

} // namespace editor
