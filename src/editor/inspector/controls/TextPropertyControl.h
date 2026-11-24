#pragma once

#include "ValuePropertyControl.h"
#include "ui/elements/InputText.h"

namespace editor
{

struct TPropertyDesc;

class CTextPropertyControl final:
  public CValuePropertyControl
{
public: // Constructor

  explicit CTextPropertyControl(const TPropertyDesc & _Desc);

protected: // CValuePropertyControl

  void OnDraw(const TPropertyValue & _Value) override;

private: // Attributes

  ui::CInputText m_Input;
};

} // namespace editor
