#include "ColorPropertyControl.h"
#include "editor/inspector/PropertyDesc.h"

namespace editor
{

//
// Constructor
//

CColorPropertyControl::CColorPropertyControl(const TPropertyDesc & _Desc):
  CValuePropertyControl(_Desc.Identifier),
  m_Edit(_Desc.Label)
{
  const auto & Meta = _Desc.Meta;

  m_Edit.SetAlphaEnabled(Meta.IsAlphaEnabled).SetEnabled(!Meta.IsReadOnly).OnChanged([this](const ImVec4 & _Colour)
  {
    SetEditedValue(Ogre::ColourValue(_Colour.x, _Colour.y, _Colour.z, _Colour.w));
  }).OnEditEnd([this]
  {
    FinishEdit();
  });
}

//
// CValuePropertyControl
//

void CColorPropertyControl::OnDraw(const TPropertyValue & _Value)
{
  if (const auto * Colour = std::get_if<Ogre::ColourValue>(&_Value))
    m_Edit.SetValue(ImVec4(Colour->r, Colour->g, Colour->b, Colour->a));

  m_Edit.Draw();
}

} // namespace editor
