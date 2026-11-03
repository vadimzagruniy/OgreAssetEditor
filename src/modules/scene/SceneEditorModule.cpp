#include "SceneEditorModule.h"
#include "SceneAssetType.h"
#include "SceneFactory.h"
#include "core/ActionQueue.h"
#include "core/EditorService.h"
#include "core/Log.h"
#include "editor/assets/AssetFactoryRegistry.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"
#include "drop/MeshDropHandler.h"
#include "drop/SceneDropHandlers.h"
#include "presenter/SceneEditorController.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"
#include "views/SceneOutlinerWindow.h"

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
  auto AssetTypes     = _Services.GetService<CAssetTypeRegistry>();
  auto AssetFactories = _Services.GetService<CAssetFactoryRegistry>();
  auto WindowManager  = _Services.GetService<ui::CWindowManager>();
  auto ActionQueue    = _Services.GetService<core::CActionQueue>();
  auto EditorLog      = _Services.GetService<core::CLog>();

  if (!AssetTypes || !AssetFactories || !WindowManager || !ActionQueue || !EditorLog)
  {
    _Error = "Scene Editor module requires the asset type registry, asset factory registry, window manager, action queue and log services.";
    return false;
  }

  m_WindowManager = std::move(WindowManager);

  auto DropHandlers = std::make_shared<CSceneDropHandlers>();
  DropHandlers->Register(std::make_shared<CMeshDropHandler>());

  auto Controller = std::make_shared<CSceneEditorController>(std::move(ActionQueue), AssetTypes, std::move(DropHandlers));

  const ui::TToolBoxDesc OutlinerDesc{GetInfo().Name, "Scene Outliner", false, ui::EDockSlot::LeftBottom, 0};
  m_WindowManager->RegisterToolBox(OutlinerDesc, std::make_shared<CSceneOutlinerWindow>(OutlinerDesc, Controller));

  AssetTypes->Register(std::make_shared<CSceneAssetType>(std::move(Controller), std::move(EditorLog)));
  AssetFactories->Register(std::make_shared<CSceneFactory>());

  return true;
}

void CSceneEditorModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CSceneEditorModule, "SceneEditor")

} // namespace editor
