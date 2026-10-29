#include "MeshViewerModule.h"
#include "MeshAssetType.h"
#include "core/EditorService.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"

namespace editor
{

//
// IModule
//

const TModuleInfo & CMeshViewerModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"MeshViewer", {}};
  return Info;
}

bool CMeshViewerModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto AssetTypes = _Services.GetService<CAssetTypeRegistry>();
  if (!AssetTypes)
  {
    _Error = "Mesh Viewer module requires the asset type registry service.";
    return false;
  }

  AssetTypes->Register(std::make_shared<CMeshAssetType>());

  return true;
}

IMPLEMENT_MODULE(CMeshViewerModule, "MeshViewer")

} // namespace editor
