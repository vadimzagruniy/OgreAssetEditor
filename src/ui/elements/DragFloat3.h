#pragma once

#include "ui/elements/ElementSetup.h"

#include <array>
#include <functional>
#include <string>

namespace editor::ui
{

class CDragFloat3 final:
  public CElementSetup<CDragFloat3>
{
public: // Types

  using TValue = std::array<float, 3>;

public: // Constructor

  explicit CDragFloat3(std::string _Label);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  float CalcLabelExtent() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CDragFloat3 & SetValue(const TValue & _Value) noexcept;

  CDragFloat3 & SetRange(float _Minimum, float _Maximum) noexcept;

  CDragFloat3 & SetSpeed(float _Speed) noexcept;

  CDragFloat3 & SetFormat(std::string _Format);

  CDragFloat3 & OnChanged(std::function<void(const TValue &)> _Callback);

public: // Interface

  const TValue & GetValue() const noexcept;

private: // Attributes

  std::string                         m_Format;
  std::function<void(const TValue &)> m_ChangeCallback;
  TValue                              m_Value;
  float                               m_Minimum;
  float                               m_Maximum;
  float                               m_Speed;
};

} // namespace editor::ui
