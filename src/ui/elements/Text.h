#pragma once

#include "ui/elements/ElementSetup.h"

#include <string>

namespace editor::ui
{

class CText final:
  public CElementSetup<CText>
{
public: // Constructor

  explicit CText(std::string _Text);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  void OnDraw(const ImVec2 & _Size) override;
};

} // namespace editor::ui
