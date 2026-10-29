#include "ModuleRegistry.h"
#include "IModule.h"

#include <utility>

namespace editor
{

//
// Access
//

CModuleRegistry & CModuleRegistry::GetInstance()
{
  static CModuleRegistry Registry;
  return Registry;
}

//
// Interface
//

bool CModuleRegistry::Register(std::string _Name, TFactory _Factory)
{
  for (const auto & Entry : m_Entries)
  {
    if (Entry.Name == _Name)
      return false;
  }

  m_Entries.push_back({std::move(_Name), std::move(_Factory)});

  return true;
}

const std::vector<CModuleRegistry::TEntry> & CModuleRegistry::GetEntries() const noexcept
{
  return m_Entries;
}

} // namespace editor
