#include "ModulesManager.h"
#include "IModule.h"
#include "ModuleInfo.h"
#include "ModuleRegistry.h"

#include <functional>
#include <unordered_map>

namespace editor
{

//
// Constructor/Destructor
//

CModulesManager::CModulesManager()
{
  // Empty
}

CModulesManager::~CModulesManager() = default;

//
// Interface
//

bool CModulesManager::Startup(core::CEditorService & _Services, std::string & _Error)
{
  Shutdown();

  _Error.clear();

  const auto & Entries = CModuleRegistry::GetInstance().GetEntries();

  std::unordered_map<std::string, std::unique_ptr<IModule>> Pending;
  for (const auto & Entry : Entries)
  {
    auto Module = Entry.Factory ? Entry.Factory() : nullptr;
    if (!Module || Module->GetInfo().Name != Entry.Name)
    {
      _Error = "Invalid module registration: " + Entry.Name;
      return false;
    }

    Pending.emplace(Entry.Name, std::move(Module));
  }

  enum class EVisitState
  {
    Unseen,
    Visiting,
    Visited
  };

  std::unordered_map<std::string, EVisitState> States;
  std::vector<std::string>                     Order;

  std::function<bool(const std::string &)> Visit = [&](const std::string & _Name)
  {
    const auto ModuleIterator = Pending.find(_Name);
    if (ModuleIterator == Pending.end())
    {
      _Error = "Missing module dependency: " + _Name;
      return false;
    }

    auto & State = States[_Name];
    if (State == EVisitState::Visited)
      return true;

    if (State == EVisitState::Visiting)
    {
      _Error = "Module dependency cycle at: " + _Name;
      return false;
    }

    State = EVisitState::Visiting;

    for (const auto & Dependency : ModuleIterator->second->GetInfo().Dependencies)
    {
      if (!Visit(Dependency))
        return false;
    }

    State = EVisitState::Visited;
    Order.push_back(_Name);

    return true;
  };

  for (const auto & Entry : Entries)
  {
    if (!Visit(Entry.Name))
      return false;
  }

  for (const auto & Name : Order)
    m_Modules.push_back(std::move(Pending.at(Name)));

  for (auto & Module : m_Modules)
  {
    ++m_StartedCount;

    if (!Module->Startup(_Services, _Error))
    {
      if (_Error.empty())
        _Error = "Module startup failed: " + Module->GetInfo().Name;

      Shutdown();
      return false;
    }
  }

  return true;
}

void CModulesManager::Shutdown() noexcept
{
  while (m_StartedCount)
    m_Modules[--m_StartedCount]->Shutdown();

  m_Modules.clear();
}

void CModulesManager::Tick(float _DeltaTime)
{
  for (auto & Module : m_Modules)
    Module->Tick(_DeltaTime);
}

} // namespace editor
