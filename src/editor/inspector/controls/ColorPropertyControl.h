#pragma once

#include "ValuePropertyControl.h"
#include "ui/elements/ColorEdit.h"

namespace editor
{

struct TPropertyDesc;

class CColorPropertyControl final:
  public CValuePropertyControl
{
public: // Constructor

  explicit CColorPropertyControl(const TPropertyDesc & _Desc);

protected: // CValuePropertyControl

  void OnDraw(const TPropertyValue & _Value) override;

private: // Attributes

  ui::CColorEdit m_Edit;
};

} // namespace editor
