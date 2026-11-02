#pragma once

#include "ui/elements/ElementSetup.h"

#include <functional>
#include <string>

namespace editor::ui
{

class CDragFloat final:
  public CElementSetup<CDragFloat>
{
public: // Constructor

  explicit CDragFloat(std::string _Label);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  float CalcLabelExtent() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CDragFloat & SetValue(float _Value) noexcept;

  CDragFloat & SetRange(float _Minimum, float _Maximum) noexcept;

  CDragFloat & SetSpeed(float _Speed) noexcept;

  CDragFloat & SetFormat(std::string _Format);

  CDragFloat & OnChanged(std::function<void(float)> _Callback);

public: // Interface

  float GetValue() const noexcept;

private: // Attributes

  std::string                m_Format;
  std::function<void(float)> m_ChangeCallback;
  float                      m_Value;
  float                      m_Minimum;
  float                      m_Maximum;
  float                      m_Speed;
};

} // namespace editor::ui
