#pragma once

#include "ui/elements/ElementSetup.h"

namespace editor::ui
{

class CSpacer final:
  public CElementSetup<CSpacer>
{
public: // Constructor

  CSpacer();

protected: // CElement

  ImVec2 CalcContentSize() const override;

  void OnDraw(const ImVec2 & _Size) override;
};

} // namespace editor::ui
