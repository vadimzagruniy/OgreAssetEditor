#pragma once

#include "core/TypeKey.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace editor::ui
{

class CWindow;
class CModalWindow;
struct TToolBoxDesc;

class CWindowManager
{
public: // Constructor/Destructor

  CWindowManager();

  ~CWindowManager();

public: // Interface

  bool RegisterToolBox(const TToolBoxDesc & _Desc, std::shared_ptr<CWindow> _Window);

  std::shared_ptr<CWindow> GetToolBox(const core::TTypeKey & _ToolBoxType) const;

  bool ShowToolBox(const core::TTypeKey & _ToolBoxType);

  bool HideToolBox(const core::TTypeKey & _ToolBoxType);

  void UnregisterModuleToolBoxes(const std::string & _Module);

  void OpenModal(std::unique_ptr<CModalWindow> _Modal);

  void Draw();

  void DrawMenuItems();

private: // Types

  struct TWindowEntry
  {
    std::string              Module;
    std::shared_ptr<CWindow> Window;
  };

private: // Attributes

  std::vector<TWindowEntry>                                    m_Windows;
  std::unordered_map<core::TTypeKey, std::shared_ptr<CWindow>> m_ToolBoxes;
  std::vector<std::unique_ptr<CModalWindow>>                   m_Modals;
};

} // namespace editor::ui
