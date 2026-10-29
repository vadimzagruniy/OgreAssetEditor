#include "ConsoleModule.h"
#include "ConsoleWindow.h"
#include "core/EditorService.h"
#include "core/Log.h"
#include "editor/modules/ModuleInfo.h"
#include "editor/modules/ModuleRegistry.h"
#include "ui/ToolBoxDesc.h"
#include "ui/WindowManager.h"

namespace editor
{

//
// IModule
//

const TModuleInfo & CConsoleModule::GetInfo() const noexcept
{
  static const TModuleInfo Info{"Console", {}};
  return Info;
}

bool CConsoleModule::Startup(core::CEditorService & _Services, std::string & _Error)
{
  auto WindowManager = _Services.GetService<ui::CWindowManager>();
  auto EditorLog     = _Services.GetService<core::CLog>();

  if (!WindowManager || !EditorLog)
  {
    _Error = "Console module requires the window manager and editor log services.";
    return false;
  }

  m_WindowManager = std::move(WindowManager);

  const ui::TToolBoxDesc Desc{GetInfo().Name, "Console", true, ui::EDockSlot::Bottom, 0};
  m_WindowManager->RegisterToolBox(Desc, std::make_shared<CConsoleWindow>(Desc, std::move(EditorLog)));

  return true;
}

void CConsoleModule::Shutdown() noexcept
{
  if (m_WindowManager)
    m_WindowManager->UnregisterModuleToolBoxes(GetInfo().Name);
}

IMPLEMENT_MODULE(CConsoleModule, "Console")

} // namespace editor
