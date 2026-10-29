#include "PropertiesModule.h"
#include "PropertiesWindow.h"
#include "core/EditorService.h"
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
  auto WindowManager    = _Services.GetService<ui::CWindowManager>();
  auto WorkspaceManager = _Services.GetService<CWorkspaceManager>();

  if (!WindowManager || !WorkspaceManager)
  {
    _Error = "Properties module requires the window manager and workspace manager services.";
    return false;
  }

  m_WindowManager = std::move(WindowManager);

  const ui::TToolBoxDesc Desc{GetInfo().Name, "Properties", true, ui::EDockSlot::Right, 0};
  m_WindowManager->RegisterToolBox(Desc, std::make_shared<CPropertiesWindow>(Desc, std::move(WorkspaceManager)));

  return true;
}

void CPropertiesModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CPropertiesModule, "Properties")

} // namespace editor
