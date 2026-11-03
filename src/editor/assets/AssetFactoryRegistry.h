#pragma once

#include <memory>
#include <string>
#include <vector>

namespace editor
{

class IAssetFactory;

class CAssetFactoryRegistry
{
public: // Interface

  void Register(std::shared_ptr<IAssetFactory> _Factory);

  std::shared_ptr<IAssetFactory> FindByIdentifier(const std::string & _Identifier) const;

  const std::vector<std::shared_ptr<IAssetFactory>> & GetFactories() const noexcept;

private: // Attributes

  std::vector<std::shared_ptr<IAssetFactory>> m_Factories;
};

} // namespace editor
