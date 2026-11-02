#include "MaterialEditorModule.h"
#include "MaterialAssetType.h"
#include "core/ActionQueue.h"
#include "core/EditorService.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"
#include "presenter/MaterialEditorController.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"
#include "views/MaterialInspectorWindow.h"
#include "views/MaterialOutlineWindow.h"

namespace editor
{

//
// IModule
//

const TModuleInfo & CMaterialEditorModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"MaterialEditor", {}};
  return Info;
}

bool CMaterialEditorModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto AssetTypes    = _Services.GetService<CAssetTypeRegistry>();
  auto AssetManager  = _Services.GetService<CAssetManager>();
  auto WindowManager = _Services.GetService<ui::CWindowManager>();
  auto ActionQueue   = _Services.GetService<core::CActionQueue>();

  if (!AssetTypes || !AssetManager || !WindowManager || !ActionQueue)
  {
    _Error = "Material Editor module requires the asset type registry, asset manager, window manager and action queue services.";
    return false;
  }

  m_WindowManager = std::move(WindowManager);

  auto Controller = std::make_shared<CMaterialEditorController>(std::move(ActionQueue));

  const ui::TToolBoxDesc OutlineDesc{GetInfo().Name, "Material Outline", false, ui::EDockSlot::LeftBottom, 0};
  m_WindowManager->RegisterToolBox(OutlineDesc, std::make_shared<CMaterialOutlineWindow>(OutlineDesc, Controller));

  const ui::TToolBoxDesc InspectorDesc{GetInfo().Name, "Material Inspector", false, ui::EDockSlot::RightBottom, 0};
  m_WindowManager->RegisterToolBox(InspectorDesc, std::make_shared<CMaterialInspectorWindow>(InspectorDesc, Controller, std::move(AssetManager)));

  AssetTypes->Register(std::make_shared<CMaterialAssetType>(std::move(Controller)));

  return true;
}

void CMaterialEditorModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CMaterialEditorModule, "MaterialEditor")

} // namespace editor
