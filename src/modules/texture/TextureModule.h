#pragma once

#include "editor/modules/IModule.h"

namespace editor
{

class CTextureModule final:
  public IModule
{
public: // IModule

  const TModuleInfo & GetInfo() const noexcept override;

  bool Startup(core::CEditorService & _Services, std::string & _Error) override;
};

} // namespace editor
