#include "FloatPropertyControl.h"
#include "editor/inspector/PropertyDesc.h"

namespace editor
{

//
// Constructor
//

CFloatPropertyControl::CFloatPropertyControl(const TPropertyDesc & _Desc):
  CValuePropertyControl(_Desc.Identifier),
  m_Drag(_Desc.Label)
{
  const auto & Meta = _Desc.Meta;

  m_Drag.SetRange(Meta.Minimum, Meta.Maximum).SetSpeed(Meta.Speed).SetFormat(Meta.Format).SetEnabled(!Meta.IsReadOnly).OnChanged([this](float _Value)
  {
    SetEditedValue(_Value);
  }).OnEditEnd([this]
  {
    FinishEdit();
  });
}

//
// CValuePropertyControl
//

void CFloatPropertyControl::OnDraw(const TPropertyValue & _Value)
{
  if (const auto * Value = std::get_if<float>(&_Value))
    m_Drag.SetValue(*Value);

  m_Drag.Draw();
}

} // namespace editor
