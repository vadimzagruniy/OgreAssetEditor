#include "MaterialEditorModule.h"
#include "MaterialAssetType.h"
#include "MaterialEditingSession.h"
#include "core/EditorService.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"
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

  if (!AssetTypes || !AssetManager || !WindowManager)
  {
    _Error = "Material Editor module requires the asset type registry, asset manager and window manager services.";
    return false;
  }

  m_WindowManager = std::move(WindowManager);

  auto Session = std::make_shared<CMaterialEditingSession>();

  const ui::TToolBoxDesc OutlineDesc{GetInfo().Name, "Material Outline", false, ui::EDockSlot::LeftBottom, 0};
  m_WindowManager->RegisterToolBox(OutlineDesc, std::make_shared<CMaterialOutlineWindow>(OutlineDesc, Session));

  const ui::TToolBoxDesc InspectorDesc{GetInfo().Name, "Material Inspector", false, ui::EDockSlot::RightBottom, 0};
  m_WindowManager->RegisterToolBox(InspectorDesc, std::make_shared<CMaterialInspectorWindow>(InspectorDesc, Session, std::move(AssetManager)));

  AssetTypes->Register(std::make_shared<CMaterialAssetType>(std::move(Session)));

  return true;
}

void CMaterialEditorModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CMaterialEditorModule, "MaterialEditor")

} // namespace editor
