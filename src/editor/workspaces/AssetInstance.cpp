#include "AssetInstance.h"
#include "AssetProperty.h"
#include "editor/assets/AssetRecord.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CAssetInstance::CAssetInstance(TAssetRecord _Record, std::string _TypeName):
  m_Record(std::make_unique<TAssetRecord>(std::move(_Record))),
  m_TypeName(std::move(_TypeName))
{
  // Empty
}

CAssetInstance::~CAssetInstance() = default;

//
// Interface
//

const TAssetRecord & CAssetInstance::GetRecord() const noexcept
{
  return *m_Record;
}

const std::string & CAssetInstance::GetTypeName() const noexcept
{
  return m_TypeName;
}

} // namespace editor
