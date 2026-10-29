#include "AssetTypeRegistry.h"
#include "AssetType.h"
#include "AssetTypeInfo.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <utility>

namespace editor
{

namespace
{

std::string NormalizeExtension(std::string _Extension)
{
  if (!_Extension.empty() && _Extension.front() != '.')
    _Extension.insert(_Extension.begin(), '.');

  std::transform(_Extension.begin(), _Extension.end(), _Extension.begin(),
                 [](unsigned char _Character) { return static_cast<char>(std::tolower(_Character)); });

  return _Extension;
}

} // namespace

//
// Interface
//

void CAssetTypeRegistry::Register(std::shared_ptr<IAssetType> _AssetType)
{
  assert(_AssetType);

  const auto & Info = _AssetType->GetInfo();
  assert(!Info.Identifier.empty());

  const auto [AssetTypeIterator, WasInserted] = m_Types.emplace(Info.Identifier, _AssetType);
  assert(WasInserted && "Asset type id already registered");

  for (const auto & Extension : Info.Extensions)
  {
    const auto [ExtensionIterator, WasExtensionInserted] = m_Extensions.emplace(NormalizeExtension(Extension), _AssetType);
    assert(WasExtensionInserted && "Asset extension already registered");
  }
}

std::shared_ptr<IAssetType> CAssetTypeRegistry::FindByExtension(std::string _Extension) const
{
  const auto ExtensionIterator = m_Extensions.find(NormalizeExtension(std::move(_Extension)));
  return ExtensionIterator == m_Extensions.end() ? nullptr : ExtensionIterator->second;
}

const std::unordered_map<std::string, std::shared_ptr<IAssetType>> & CAssetTypeRegistry::GetTypes() const noexcept
{
  return m_Types;
}

} // namespace editor
