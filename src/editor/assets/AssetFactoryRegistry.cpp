#include "AssetFactoryRegistry.h"
#include "AssetFactory.h"
#include "AssetFactoryInfo.h"

#include <algorithm>
#include <cassert>
#include <utility>

namespace editor
{

//
// Interface
//

void CAssetFactoryRegistry::Register(std::shared_ptr<IAssetFactory> _Factory)
{
  assert(_Factory);
  assert(!_Factory->GetInfo().Identifier.empty());
  assert(!FindByIdentifier(_Factory->GetInfo().Identifier) && "Asset factory id already registered");

  m_Factories.push_back(std::move(_Factory));
}

std::shared_ptr<IAssetFactory> CAssetFactoryRegistry::FindByIdentifier(const std::string & _Identifier) const
{
  const auto FactoryIterator = std::find_if(m_Factories.begin(), m_Factories.end(), [&_Identifier](const std::shared_ptr<IAssetFactory> & _Factory)
  {
    return _Factory->GetInfo().Identifier == _Identifier;
  });

  return FactoryIterator == m_Factories.end() ? nullptr : *FactoryIterator;
}

const std::vector<std::shared_ptr<IAssetFactory>> & CAssetFactoryRegistry::GetFactories() const noexcept
{
  return m_Factories;
}

} // namespace editor
