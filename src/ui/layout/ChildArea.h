#pragma once

#include "ui/elements/ElementSetup.h"
#include "ui/layout/Layout.h"

#include <string>

namespace editor::ui
{

class CChildArea final:
  public CElementSetup<CChildArea>
{
public: // Constructor

  CChildArea(std::string _Id, ELayoutDirection _Direction);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CChildArea & SetWindowFlags(ImGuiWindowFlags _Flags) noexcept;

  CChildArea & SetScrollToEnd(bool _IsScrollToEnd) noexcept;

public: // Interface

  CLayout & GetContent() noexcept;

private: // Service

  void ScrollToEnd() const;

private: // Attributes

  CLayout          m_Content;
  ImGuiWindowFlags m_WindowFlags;
  bool             m_IsScrollToEnd;
};

} // namespace editor::ui
