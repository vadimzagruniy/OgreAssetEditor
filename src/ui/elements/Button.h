#pragma once

#include "ui/elements/ElementSetup.h"

#include <functional>
#include <string>

namespace editor::ui
{

class CButton final:
  public CElementSetup<CButton>
{
public: // Constructor

  explicit CButton(std::string _Label);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CButton & OnClick(std::function<void()> _Callback);

private: // Attributes

  std::function<void()> m_ClickCallback;
};

} // namespace editor::ui
