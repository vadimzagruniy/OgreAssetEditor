#pragma once

#include "ui/elements/ShapeSetup.h"

namespace editor::ui
{

class CCircle final:
  public CShapeSetup<CCircle>
{
public: // Constructor

  CCircle();

protected: // CElement

  void OnDraw(const ImVec2 & _Size) override;
};

} // namespace editor::ui
