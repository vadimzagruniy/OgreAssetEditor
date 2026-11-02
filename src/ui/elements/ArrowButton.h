#pragma once

#include "ui/elements/ElementSetup.h"

#include <functional>
#include <string>

namespace editor::ui
{

class CArrowButton final:
  public CElementSetup<CArrowButton>
{
public: // Constructor

  CArrowButton(std::string _Label, ImGuiDir _Direction);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CArrowButton & OnClick(std::function<void()> _Callback);

private: // Attributes

  std::function<void()> m_ClickCallback;
  ImGuiDir              m_Direction;
};

} // namespace editor::ui
