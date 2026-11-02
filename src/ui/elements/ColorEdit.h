#pragma once

#include "ui/elements/ElementSetup.h"

#include <functional>
#include <string>

namespace editor::ui
{

class CColorEdit final:
  public CElementSetup<CColorEdit>
{
public: // Constructor

  explicit CColorEdit(std::string _Label);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CColorEdit & SetValue(const ImVec4 & _Value) noexcept;

  CColorEdit & SetAlphaEnabled(bool _IsAlphaEnabled) noexcept;

  CColorEdit & OnChanged(std::function<void(const ImVec4 &)> _Callback);

public: // Interface

  const ImVec4 & GetValue() const noexcept;

private: // Attributes

  std::function<void(const ImVec4 &)> m_ChangeCallback;
  ImVec4                              m_Value;
  bool                                m_IsAlphaEnabled;
};

} // namespace editor::ui
