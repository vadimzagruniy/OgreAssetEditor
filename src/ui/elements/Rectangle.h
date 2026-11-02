#pragma once

#include "ui/elements/ShapeSetup.h"

namespace editor::ui
{

class CRectangle final:
  public CShapeSetup<CRectangle>
{
public: // Constructor

  CRectangle();

protected: // CElement

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CRectangle & SetRounding(float _Rounding) noexcept;

private: // Attributes

  float m_Rounding;
};

} // namespace editor::ui
