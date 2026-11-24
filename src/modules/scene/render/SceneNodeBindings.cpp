#include "SceneNodeBindings.h"
#include "ISceneNodeBinding.h"

#include <cassert>
#include <utility>

namespace editor
{

//
// Interface
//

std::unique_ptr<ISceneNodeBinding> CSceneNodeBindings::Create(const std::string & _TypeId, const TSceneBindingContext & _Context) const
{
  const auto FactoryIterator = m_Factories.find(_TypeId);
  return FactoryIterator == m_Factories.end() ? nullptr : FactoryIterator->second(_Context);
}

//
// Implementation
//

void CSceneNodeBindings::Register(std::string _TypeId, TFactory _Factory)
{
  const auto [FactoryIterator, WasInserted] = m_Factories.emplace(std::move(_TypeId), std::move(_Factory));
  assert(WasInserted && "Scene node binding already registered");
  (void)FactoryIterator;
}

} // namespace editor
