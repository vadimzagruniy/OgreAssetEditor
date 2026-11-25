#include "AnimationEditorModule.h"
#include "AnimationAssetType.h"
#include "AnimationFactory.h"
#include "core/ActionQueue.h"
#include "core/EditorService.h"
#include "editor/assets/AssetFactoryRegistry.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"
#include "views/AnimationTimelineWindow.h"

#include <utility>

namespace editor
{

//
// IModule
//

const TModuleInfo & CAnimationEditorModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"AnimationEditor", {"SceneEditor"}};
  return Info;
}

bool CAnimationEditorModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto AssetTypes       = _Services.GetService<CAssetTypeRegistry>();
  auto AssetFactories   = _Services.GetService<CAssetFactoryRegistry>();
  auto WindowManager    = _Services.GetService<ui::CWindowManager>();
  auto WorkspaceManager = _Services.GetService<CWorkspaceManager>();
  auto ActionQueue      = _Services.GetService<core::CActionQueue>();

  if (!AssetTypes || !AssetFactories || !WindowManager || !WorkspaceManager || !ActionQueue)
  {
    _Error = "Animation Editor module requires the asset type registry, asset factory registry, window manager, workspace manager and action queue services.";
    return false;
  }

  m_WindowManager = std::move(WindowManager);

  const ui::TToolBoxDesc TimelineDesc{GetInfo().Name, "Timeline", false, ui::EDockSlot::Bottom, 0};
  auto                   TimelineWindow = std::make_shared<CAnimationTimelineWindow>(TimelineDesc, WorkspaceManager, m_WindowManager);

  m_WindowManager->RegisterToolBox(TimelineDesc, TimelineWindow);

  AssetTypes->Register(std::make_shared<CAnimationAssetType>(std::move(WorkspaceManager), std::move(TimelineWindow), std::move(ActionQueue)));
  AssetFactories->Register(std::make_shared<CAnimationFactory>());

  return true;
}

void CAnimationEditorModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CAnimationEditorModule, "AnimationEditor")

} // namespace editor
