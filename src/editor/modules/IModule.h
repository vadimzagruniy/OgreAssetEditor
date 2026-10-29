#pragma once

#include <string>

namespace editor
{

namespace core
{
class CEditorService;
} // namespace core

struct TModuleInfo;

class IModule
{
public: // Destructor

  virtual ~IModule();

public: // Interface

  virtual const TModuleInfo & GetInfo() const noexcept = 0;

  virtual bool Startup(core::CEditorService & _Services, std::string & _Error) = 0;

  virtual void Shutdown() noexcept;

  virtual void Tick(float _DeltaTime);
};

} // namespace editor
