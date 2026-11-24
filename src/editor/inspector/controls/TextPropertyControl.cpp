#include "TextPropertyControl.h"
#include "editor/inspector/PropertyDesc.h"

namespace editor
{

//
// Constructor
//

CTextPropertyControl::CTextPropertyControl(const TPropertyDesc & _Desc):
  CValuePropertyControl(_Desc.Identifier),
  m_Input(_Desc.Label)
{
  m_Input.SetHint(_Desc.Meta.Hint).SetEnabled(!_Desc.Meta.IsReadOnly).OnChanged([this](const std::string & _Text)
  {
    SetEditedValue(_Text);
  }).OnEditEnd([this]
  {
    FinishEdit();
  });
}

//
// CValuePropertyControl
//

void CTextPropertyControl::OnDraw(const TPropertyValue & _Value)
{
  const auto * Text = std::get_if<std::string>(&_Value);
  m_Input.SetText(Text ? *Text : std::string());

  m_Input.Draw();
}

} // namespace editor
