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

bool IAssetType::ReplaceDependency(const TAssetRecord & /*_Asset*/, const TAssetReference & /*_OldRef*/, const TAssetReference & /*_NewRef*/, bool & _IsChanged, std::string & /*_Error*/) const
{
  _IsChanged = false;
  return true;
}

} // namespace editor
