#include "EnumPropertyControl.h"
#include "editor/inspector/PropertyDesc.h"

#include <optional>

namespace editor
{

//
// Constructor
//

CEnumPropertyControl::CEnumPropertyControl(const TPropertyDesc & _Desc):
  CValuePropertyControl(_Desc.Identifier),
  m_Combo(_Desc.Label)
{
  const auto & Meta = _Desc.Meta;

  m_Combo.SetItems(Meta.Options).SetPlaceholder(Meta.Hint).SetEnabled(!Meta.IsReadOnly).OnSelected([this](std::size_t _OptionIndex)
  {
    SetEditedValue(_OptionIndex);
    FinishEdit();
  });
}

//
// CValuePropertyControl
//

void CEnumPropertyControl::OnDraw(const TPropertyValue & _Value)
{
  const auto * OptionIndex = std::get_if<std::size_t>(&_Value);
  m_Combo.SetSelectedIndex(OptionIndex ? std::optional<std::size_t>(*OptionIndex) : std::nullopt);

  m_Combo.Draw();
}

} // namespace editor
