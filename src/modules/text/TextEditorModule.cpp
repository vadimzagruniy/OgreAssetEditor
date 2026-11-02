#include "TextEditorModule.h"
#include "TextAssetType.h"
#include "core/EditorService.h"
#include "core/Log.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"

#include <utility>

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
  auto EditorLog  = _Services.GetService<core::CLog>();

  if (!AssetTypes || !EditorLog)
  {
    _Error = "Text Editor module requires the asset type registry and log services.";
    return false;
  }

  AssetTypes->Register(std::make_shared<CTextAssetType>(std::move(EditorLog)));

  return true;
}

IMPLEMENT_MODULE(CTextEditorModule, "TextEditor")

} // namespace editor
