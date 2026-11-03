#pragma once

#include "IPropertyControl.h"
#include "editor/inspector/PropertyValue.h"

#include <optional>
#include <string>

namespace editor
{

class CValuePropertyControl:
  public IPropertyControl
{
protected: // Constructor

  explicit CValuePropertyControl(std::string _Identifier);

public: // IPropertyControl

  void Draw(IPropertySource & _Source) final;

protected: // Service

  void SetEditedValue(TPropertyValue _Value);

  void FinishEdit() noexcept;

protected: // Events

  virtual void OnDraw(const TPropertyValue & _Value) = 0;

private: // Attributes

  std::string                   m_Identifier;
  std::optional<TPropertyValue> m_EditedValue;
  bool                          m_IsEditFinished;
};

} // namespace editor
