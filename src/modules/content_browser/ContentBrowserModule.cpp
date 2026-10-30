#include "ContentBrowserModule.h"
#include "ContentBrowserWindow.h"
#include "core/EditorService.h"
#include "core/Log.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"
#include "editor/workspaces/WorkspaceManager.h"
#include "presenter/ContentBrowserController.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"

namespace editor
{

//
// IModule
//

const TModuleInfo & CContentBrowserModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"ContentBrowser", {}};
  return Info;
}

bool CContentBrowserModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto WindowManager    = _Services.GetService<ui::CWindowManager>();
  auto AssetManager     = _Services.GetService<CAssetManager>();
  auto WorkspaceManager = _Services.GetService<CWorkspaceManager>();
  auto AssetTypes       = _Services.GetService<CAssetTypeRegistry>();
  auto EditorLog        = _Services.GetService<core::CLog>();

  if (!WindowManager || !AssetManager || !WorkspaceManager || !AssetTypes || !EditorLog)
  {
    _Error = "Content Browser module requires the window manager, asset manager, workspace manager, asset type registry and log services.";
    return false;
  }

  auto Controller = std::make_unique<CContentBrowserController>(std::move(AssetManager), std::move(AssetTypes), std::move(WorkspaceManager), std::move(EditorLog));

  m_WindowManager = std::move(WindowManager);

  const ui::TToolBoxDesc Desc{GetInfo().Name, "Content", true, ui::EDockSlot::Left, 0, true};
  m_WindowManager->RegisterToolBox(Desc, std::make_shared<CContentBrowserWindow>(Desc, std::move(Controller)));

  return true;
}

void CContentBrowserModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CContentBrowserModule, "ContentBrowser")

} // namespace editor
