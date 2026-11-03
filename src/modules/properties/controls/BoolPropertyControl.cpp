#include "BoolPropertyControl.h"
#include "editor/inspector/PropertyDesc.h"

namespace editor
{

//
// Constructor
//

CBoolPropertyControl::CBoolPropertyControl(const TPropertyDesc & _Desc):
  CValuePropertyControl(_Desc.Identifier),
  m_CheckBox(_Desc.Label)
{
  m_CheckBox.SetEnabled(!_Desc.Meta.IsReadOnly).OnToggled([this](bool _IsChecked)
  {
    SetEditedValue(_IsChecked);
    FinishEdit();
  });
}

//
// CValuePropertyControl
//

void CBoolPropertyControl::OnDraw(const TPropertyValue & _Value)
{
  const auto * IsChecked = std::get_if<bool>(&_Value);
  m_CheckBox.SetChecked(IsChecked && *IsChecked);

  m_CheckBox.Draw();
}

} // namespace editor
