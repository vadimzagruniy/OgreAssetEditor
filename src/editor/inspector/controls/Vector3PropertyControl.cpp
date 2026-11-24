#include "Vector3PropertyControl.h"
#include "editor/inspector/PropertyDesc.h"

namespace editor
{

//
// Constructor
//

CVector3PropertyControl::CVector3PropertyControl(const TPropertyDesc & _Desc):
  CValuePropertyControl(_Desc.Identifier),
  m_Drag(_Desc.Label)
{
  const auto & Meta = _Desc.Meta;

  m_Drag.SetRange(Meta.Minimum, Meta.Maximum).SetSpeed(Meta.Speed).SetFormat(Meta.Format).SetEnabled(!Meta.IsReadOnly).OnChanged([this](const ui::CDragFloat3::TValue & _Components)
  {
    SetEditedValue(Ogre::Vector3(_Components[0], _Components[1], _Components[2]));
  }).OnEditEnd([this]
  {
    FinishEdit();
  });
}

//
// CValuePropertyControl
//

void CVector3PropertyControl::OnDraw(const TPropertyValue & _Value)
{
  if (const auto * Vector = std::get_if<Ogre::Vector3>(&_Value))
    m_Drag.SetValue({Vector->x, Vector->y, Vector->z});

  m_Drag.Draw();
}

} // namespace editor
