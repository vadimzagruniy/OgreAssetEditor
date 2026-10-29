#pragma once

#include "DockLayoutDesc.h"

#include <string>
#include <vector>

namespace editor::ui
{

enum class EDockSlot;

using TDockId = unsigned int;

class CDockLayoutBuilder
{
public: // Constructor

  explicit CDockLayoutBuilder(const TDockLayoutDesc & _Desc);

public: // Interface

  static bool HasLayout(TDockId _DockSpaceId);

  void DockWindow(std::string _WindowLabel, EDockSlot _Slot);

  void Build(TDockId _DockSpaceId) const;

private: // Implementation

  bool IsSlotUsed(EDockSlot _Slot) const;

private: // Types

  struct TDockRequest
  {
    std::string WindowLabel;
    EDockSlot   Slot;
  };

private: // Attributes

  TDockLayoutDesc           m_Desc;
  std::vector<TDockRequest> m_Requests;
};

} // namespace editor::ui
