#pragma once

#include "ui/elements/ElementSetup.h"

#include <functional>
#include <string>

namespace editor::ui
{

class CCheckBox final:
  public CElementSetup<CCheckBox>
{
public: // Constructor

  explicit CCheckBox(std::string _Label);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CCheckBox & SetChecked(bool _IsChecked) noexcept;

  CCheckBox & OnToggled(std::function<void(bool)> _Callback);

public: // Interface

  bool IsChecked() const noexcept;

private: // Attributes

  std::function<void(bool)> m_ToggleCallback;
  bool                      m_IsChecked;
};

} // namespace editor::ui
