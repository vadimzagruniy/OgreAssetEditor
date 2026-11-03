#include "PropertiesModule.h"
#include "PropertiesWindow.h"
#include "core/ActionQueue.h"
#include "core/EditorService.h"
#include "core/Log.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"

namespace editor
{

//
// IModule
//

const TModuleInfo & CPropertiesModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"Properties", {"ContentBrowser"}};
  return Info;
}

bool CPropertiesModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto WindowManager = _Services.GetService<ui::CWindowManager>();

  TPropertyControlContext Context;
  Context.AssetManager     = _Services.GetService<CAssetManager>();
  Context.AssetTypes       = _Services.GetService<CAssetTypeRegistry>();
  Context.WorkspaceManager = _Services.GetService<CWorkspaceManager>();
  Context.ActionQueue      = _Services.GetService<core::CActionQueue>();
  Context.Log              = _Services.GetService<core::CLog>();

  if (!WindowManager || !Context.AssetManager || !Context.AssetTypes || !Context.WorkspaceManager || !Context.ActionQueue || !Context.Log)
  {
    _Error = "Properties module requires the window manager, asset manager, asset type registry, workspace manager, action queue and log services.";
    return false;
  }

  m_WindowManager = std::move(WindowManager);

  const ui::TToolBoxDesc Desc{GetInfo().Name, "Properties", true, ui::EDockSlot::Right, 0};
  m_WindowManager->RegisterToolBox(Desc, std::make_shared<CPropertiesWindow>(Desc, std::move(Context)));

  return true;
}

void CPropertiesModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CPropertiesModule, "Properties")

} // namespace editor
