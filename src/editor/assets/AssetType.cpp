#include "AssetType.h"
#include "AssetMenuEntry.h"

namespace editor
{

//
// Destructor
//

IAssetType::~IAssetType() = default;

//
// Interface
//

void IAssetType::GetContextActions(const TAssetRecord & /*_Asset*/, std::vector<TAssetMenuEntry> & /*_Entries*/) const
{
  // Empty
}

bool IAssetType::CollectDependencies(const TAssetRecord & /*_Asset*/, std::vector<TAssetReference> & /*_DependencyRefs*/, std::string & /*_Error*/) const
{
  return true;
}

} // namespace editor
