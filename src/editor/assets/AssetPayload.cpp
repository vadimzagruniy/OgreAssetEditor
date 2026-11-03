#include "AssetPayload.h"
#include "ui/DragPayload.h"

namespace editor
{

const ui::CDragPayload & GetAssetPayload()
{
  static const ui::CDragPayload Payload("ASSET_PATH");
  return Payload;
}

} // namespace editor
