#pragma once

#include "ui/elements/ElementSetup.h"

#include <optional>
#include <string>
#include <utility>

namespace editor::ui
{

template <class TDerived>
class CShapeSetup:
  public CElementSetup<TDerived>
{
protected: // Constructor

  CShapeSetup();

protected: // CElement

  ImVec2 CalcContentSize() const override;

public: // Setup

  TDerived & SetFillColor(ImU32 _Color) noexcept;

  TDerived & SetOutlineColor(ImU32 _Color) noexcept;

  TDerived & SetThickness(float _Thickness) noexcept;

protected: // Interface

  std::optional<ImU32> GetFillColor() const;

  std::optional<ImU32> GetOutlineColor() const noexcept;

  float GetThickness() const noexcept;

  ImVec2 ReserveArea(const ImVec2 & _Size) const;

private: // Service

  TDerived & GetSelf() noexcept;

private: // Attributes

  std::optional<ImU32> m_FillColor;
  std::optional<ImU32> m_OutlineColor;
  float                m_Thickness;
};

//
// Template implementation
//

template <class TDerived>
CShapeSetup<TDerived>::CShapeSetup():
  CElementSetup<TDerived>(std::string()),
  m_Thickness(1.0f)
{
  // Empty
}

template <class TDerived>
ImVec2 CShapeSetup<TDerived>::CalcContentSize() const
{
  const float FrameHeight = ImGui::GetFrameHeight();

  return ImVec2(FrameHeight, FrameHeight);
}

template <class TDerived>
TDerived & CShapeSetup<TDerived>::SetFillColor(ImU32 _Color) noexcept
{
  m_FillColor = _Color;
  return GetSelf();
}

template <class TDerived>
TDerived & CShapeSetup<TDerived>::SetOutlineColor(ImU32 _Color) noexcept
{
  m_OutlineColor = _Color;
  return GetSelf();
}

template <class TDerived>
TDerived & CShapeSetup<TDerived>::SetThickness(float _Thickness) noexcept
{
  m_Thickness = _Thickness;
  return GetSelf();
}

template <class TDerived>
std::optional<ImU32> CShapeSetup<TDerived>::GetFillColor() const
{
  if (!m_FillColor && !m_OutlineColor)
    return ImGui::GetColorU32(ImGuiCol_Text);

  return m_FillColor;
}

template <class TDerived>
std::optional<ImU32> CShapeSetup<TDerived>::GetOutlineColor() const noexcept
{
  return m_OutlineColor;
}

template <class TDerived>
float CShapeSetup<TDerived>::GetThickness() const noexcept
{
  return m_Thickness;
}

template <class TDerived>
ImVec2 CShapeSetup<TDerived>::ReserveArea(const ImVec2 & _Size) const
{
  const ImVec2 Origin = ImGui::GetCursorScreenPos();
  ImGui::Dummy(_Size);

  return Origin;
}

template <class TDerived>
TDerived & CShapeSetup<TDerived>::GetSelf() noexcept
{
  return static_cast<TDerived &>(*this);
}

} // namespace editor::ui
