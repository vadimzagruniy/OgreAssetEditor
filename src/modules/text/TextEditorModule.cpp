#include "TextEditorModule.h"
#include "TextAssetType.h"
#include "core/EditorService.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"

namespace editor
{

//
// IModule
//

const TModuleInfo & CTextEditorModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"TextEditor", {}};
  return Info;
}

bool CTextEditorModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto AssetTypes = _Services.GetService<CAssetTypeRegistry>();
  if (!AssetTypes)
  {
    _Error = "Text Editor module requires the asset type registry service.";
    return false;
  }

  AssetTypes->Register(std::make_shared<CTextAssetType>());

  return true;
}

IMPLEMENT_MODULE(CTextEditorModule, "TextEditor")

} // namespace editor
