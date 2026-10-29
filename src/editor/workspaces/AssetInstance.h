#pragma once

#include <memory>
#include <string>
#include <vector>

namespace editor
{

struct TAssetRecord;
struct TAssetProperty;

class CAssetInstance
{
public: // Constructor/Destructor

  CAssetInstance(TAssetRecord _Record, std::string _TypeName);

  virtual ~CAssetInstance();

public: // Interface

  const TAssetRecord & GetRecord() const noexcept;

  const std::string & GetTypeName() const noexcept;

  virtual std::vector<TAssetProperty> GetProperties() const = 0;

private: // Attributes

  std::unique_ptr<TAssetRecord> m_Record;
  std::string                   m_TypeName;
};

} // namespace editor
