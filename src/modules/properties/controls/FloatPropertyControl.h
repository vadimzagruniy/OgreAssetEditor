#pragma once

#include "ValuePropertyControl.h"
#include "ui/elements/DragFloat.h"

namespace editor
{

struct TPropertyDesc;

class CFloatPropertyControl final:
  public CValuePropertyControl
{
public: // Constructor

  explicit CFloatPropertyControl(const TPropertyDesc & _Desc);

protected: // CValuePropertyControl

  void OnDraw(const TPropertyValue & _Value) override;

private: // Attributes

  ui::CDragFloat m_Drag;
};

} // namespace editor
