#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace editor
{

class IModule;

class CModuleRegistry
{
public: // Types

  using TFactory = std::function<std::unique_ptr<IModule>()>;

  struct TEntry
  {
    std::string Name;
    TFactory    Factory;
  };

public: // Access

  static CModuleRegistry & GetInstance();

public: // Interface

  bool Register(std::string _Name, TFactory _Factory);

  const std::vector<TEntry> & GetEntries() const noexcept;

private: // Attributes

  std::vector<TEntry> m_Entries;
};

} // namespace editor

// Registers a module factory during static initialization.
// Must be used inside namespace editor in the module's .cpp file.
#define EDITOR_JOIN_IMPL(_Left, _Right) _Left##_Right
#define EDITOR_JOIN(_Left, _Right) EDITOR_JOIN_IMPL(_Left, _Right)
#define IMPLEMENT_MODULE(ModuleClass, ModuleName)                                                                      \
  namespace                                                                                                            \
  {                                                                                                                    \
  const bool EDITOR_JOIN(g_EditorModuleRegistered_, __COUNTER__) =                                                     \
    ::editor::CModuleRegistry::GetInstance().Register(ModuleName, [] { return std::make_unique<ModuleClass>(); });     \
  }
