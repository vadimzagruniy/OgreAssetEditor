#include "SceneEditorModule.h"
#include "SceneAssetType.h"
#include "core/EditorService.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"

namespace editor
{

//
// IModule
//

const TModuleInfo & CSceneEditorModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"SceneEditor", {}};
  return Info;
}

bool CSceneEditorModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto AssetTypes = _Services.GetService<CAssetTypeRegistry>();
  if (!AssetTypes)
  {
    _Error = "Scene Editor module requires the asset type registry service.";
    return false;
  }

  AssetTypes->Register(std::make_shared<CSceneAssetType>());

  return true;
}

IMPLEMENT_MODULE(CSceneEditorModule, "SceneEditor")

} // namespace editor
