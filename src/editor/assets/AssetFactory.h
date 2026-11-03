#pragma once

#include <string>

namespace editor
{

struct TAssetFactoryInfo;

class IAssetFactory
{
public: // Destructor

  virtual ~IAssetFactory();

public: // Interface

  virtual const TAssetFactoryInfo & GetInfo() const noexcept = 0;

  virtual bool CreateContent(std::string & _Content, std::string & _Error) const = 0;
};

} // namespace editor
