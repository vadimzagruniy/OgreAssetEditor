#include "TextureModule.h"
#include "TextureAssetType.h"
#include "core/EditorService.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"

namespace editor
{

//
// IModule
//

const TModuleInfo & CTextureModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"Texture", {}};
  return Info;
}

bool CTextureModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto AssetTypes = _Services.GetService<CAssetTypeRegistry>();
  if (!AssetTypes)
  {
    _Error = "Texture module requires the asset type registry service.";
    return false;
  }

  AssetTypes->Register(std::make_shared<CTextureAssetType>());

  return true;
}

IMPLEMENT_MODULE(CTextureModule, "Texture")

} // namespace editor
