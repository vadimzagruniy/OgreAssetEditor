#pragma once

#include "ValuePropertyControl.h"
#include "ui/elements/DragFloat3.h"

namespace editor
{

struct TPropertyDesc;

class CVector3PropertyControl final:
  public CValuePropertyControl
{
public: // Constructor

  explicit CVector3PropertyControl(const TPropertyDesc & _Desc);

protected: // CValuePropertyControl

  void OnDraw(const TPropertyValue & _Value) override;

private: // Attributes

  ui::CDragFloat3 m_Drag;
};

} // namespace editor
