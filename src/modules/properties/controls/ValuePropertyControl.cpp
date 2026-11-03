#include "ValuePropertyControl.h"
#include "editor/inspector/IPropertySource.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor
//

CValuePropertyControl::CValuePropertyControl(std::string _Identifier):
  m_Identifier(std::move(_Identifier)),
  m_IsEditFinished(false)
{
  // Empty
}

//
// IPropertyControl
//

void CValuePropertyControl::Draw(IPropertySource & _Source)
{
  ImGui::PushID(m_Identifier.c_str());
  OnDraw(_Source.GetValue(m_Identifier));
  ImGui::PopID();

  if (m_EditedValue)
  {
    _Source.Preview(m_Identifier, *m_EditedValue);
    m_EditedValue.reset();
  }

  if (m_IsEditFinished)
  {
    m_IsEditFinished = false;
    _Source.Commit();
  }
}

//
// Service
//

void CValuePropertyControl::SetEditedValue(TPropertyValue _Value)
{
  m_EditedValue = std::move(_Value);
}

void CValuePropertyControl::FinishEdit() noexcept
{
  m_IsEditFinished = true;
}

} // namespace editor
