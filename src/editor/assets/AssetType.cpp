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

} // namespace editor
