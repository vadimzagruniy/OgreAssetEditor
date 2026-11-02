#pragma once

#include "ui/elements/Element.h"

#include <string>
#include <utility>

namespace editor::ui
{

template <class TDerived>
class CElementSetup:
  public CElement
{
protected: // Constructor

  explicit CElementSetup(std::string _Label);

public: // Setup

  TDerived & SetLabel(std::string _Label);

  TDerived & SetVisible(bool _IsVisible) noexcept;

  TDerived & SetEnabled(bool _IsEnabled) noexcept;

  TDerived & SetTooltip(std::string _Tooltip);

  TDerived & SetWidth(float _Width) noexcept;

  TDerived & SetWidth(const TSizeRule & _Width) noexcept;

  TDerived & SetHeight(float _Height) noexcept;

  TDerived & SetHeight(const TSizeRule & _Height) noexcept;

  TDerived & SetColor(ImGuiCol _Index, const ImVec4 & _Color);

  TDerived & SetStyleVar(ImGuiStyleVar _Index, float _Value);

  TDerived & SetStyleVar(ImGuiStyleVar _Index, const ImVec2 & _Value);

private: // Service

  TDerived & GetSelf() noexcept;
};

//
// Template implementation
//

template <class TDerived>
CElementSetup<TDerived>::CElementSetup(std::string _Label):
  CElement(std::move(_Label))
{
  // Empty
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetLabel(std::string _Label)
{
  CElement::SetLabel(std::move(_Label));
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetVisible(bool _IsVisible) noexcept
{
  CElement::SetVisible(_IsVisible);
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetEnabled(bool _IsEnabled) noexcept
{
  CElement::SetEnabled(_IsEnabled);
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetTooltip(std::string _Tooltip)
{
  CElement::SetTooltip(std::move(_Tooltip));
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetWidth(float _Width) noexcept
{
  CElement::SetWidth({ESizeMode::Fixed, _Width});
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetWidth(const TSizeRule & _Width) noexcept
{
  CElement::SetWidth(_Width);
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetHeight(float _Height) noexcept
{
  CElement::SetHeight({ESizeMode::Fixed, _Height});
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetHeight(const TSizeRule & _Height) noexcept
{
  CElement::SetHeight(_Height);
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetColor(ImGuiCol _Index, const ImVec4 & _Color)
{
  CElement::SetColor(_Index, _Color);
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetStyleVar(ImGuiStyleVar _Index, float _Value)
{
  CElement::SetStyleVar(_Index, _Value);
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::SetStyleVar(ImGuiStyleVar _Index, const ImVec2 & _Value)
{
  CElement::SetStyleVar(_Index, _Value);
  return GetSelf();
}

template <class TDerived>
TDerived & CElementSetup<TDerived>::GetSelf() noexcept
{
  return static_cast<TDerived &>(*this);
}

} // namespace editor::ui
