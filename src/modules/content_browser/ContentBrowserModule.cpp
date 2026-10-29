#include "ContentBrowserModule.h"
#include "ContentBrowserWindow.h"
#include "core/EditorService.h"
#include "editor/assets/AssetManager.h"
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

  if (!WindowManager || !AssetManager || !WorkspaceManager)
  {
    _Error = "Content Browser module requires the window manager, asset manager, and workspace manager services.";
    return false;
  }

  m_WindowManager = std::move(WindowManager);

  const ui::TToolBoxDesc Desc{GetInfo().Name, "Content", true, ui::EDockSlot::Left, 0};
  m_WindowManager->RegisterToolBox(Desc, std::make_shared<CContentBrowserWindow>(Desc, std::move(AssetManager), std::move(WorkspaceManager)));

  return true;
}

void CContentBrowserModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CContentBrowserModule, "ContentBrowser")

} // namespace editor
