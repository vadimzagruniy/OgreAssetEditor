#pragma once

#include "ValuePropertyControl.h"
#include "ui/elements/ComboBox.h"

namespace editor
{

struct TPropertyDesc;

class CEnumPropertyControl final:
  public CValuePropertyControl
{
public: // Constructor

  explicit CEnumPropertyControl(const TPropertyDesc & _Desc);

protected: // CValuePropertyControl

  void OnDraw(const TPropertyValue & _Value) override;

private: // Attributes

  ui::CComboBox m_Combo;
};

} // namespace editor
