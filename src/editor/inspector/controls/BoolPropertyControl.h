#pragma once

#include "ValuePropertyControl.h"
#include "ui/elements/CheckBox.h"

namespace editor
{

struct TPropertyDesc;

class CBoolPropertyControl final:
  public CValuePropertyControl
{
public: // Constructor

  explicit CBoolPropertyControl(const TPropertyDesc & _Desc);

protected: // CValuePropertyControl

  void OnDraw(const TPropertyValue & _Value) override;

private: // Attributes

  ui::CCheckBox m_CheckBox;
};

} // namespace editor
