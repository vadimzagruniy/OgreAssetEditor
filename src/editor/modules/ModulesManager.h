#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace editor
{

namespace core
{
class CEditorService;
} // namespace core

class IModule;

class CModulesManager
{
public: // Constructor/Destructor

  CModulesManager();

  ~CModulesManager();

public: // Interface

  bool Startup(core::CEditorService & _Services, std::string & _Error);

  void Shutdown() noexcept;

  void Tick(float _DeltaTime);

private: // Attributes

  std::vector<std::unique_ptr<IModule>> m_Modules;
  std::size_t                           m_StartedCount = 0;
};

} // namespace editor
