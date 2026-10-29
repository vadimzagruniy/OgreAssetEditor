#pragma once

#include "editor/modules/IModule.h"

#include <memory>

namespace editor
{

namespace ui
{
class CWindowManager;
} // namespace ui

class CConsoleModule final:
  public IModule
{
public: // IModule

  const TModuleInfo & GetInfo() const noexcept override;

  bool Startup(core::CEditorService & _Services, std::string & _Error) override;

  void Shutdown() noexcept override;

private: // Attributes

  std::shared_ptr<ui::CWindowManager> m_WindowManager;
};

} // namespace editor
