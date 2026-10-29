#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace editor
{

class IAssetType;

class CAssetTypeRegistry
{
public: // Interface

  void Register(std::shared_ptr<IAssetType> _AssetType);

  std::shared_ptr<IAssetType> FindByExtension(std::string _Extension) const;

  const std::unordered_map<std::string, std::shared_ptr<IAssetType>> & GetTypes() const noexcept;

private: // Attributes

  std::unordered_map<std::string, std::shared_ptr<IAssetType>> m_Types;
  std::unordered_map<std::string, std::shared_ptr<IAssetType>> m_Extensions;
};

} // namespace editor
