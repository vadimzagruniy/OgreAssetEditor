#pragma once

#include "ui/elements/ElementSetup.h"

#include <functional>
#include <string>

namespace editor::ui
{

class CInputText final:
  public CElementSetup<CInputText>
{
public: // Constructor

  explicit CInputText(std::string _Label);

protected: // CElement

  ImVec2 CalcContentSize() const override;

  float CalcLabelExtent() const override;

  void OnDraw(const ImVec2 & _Size) override;

public: // Setup

  CInputText & SetText(std::string _Text);

  CInputText & SetHint(std::string _Hint);

  CInputText & OnChanged(std::function<void(const std::string &)> _Callback);

public: // Interface

  const std::string & GetText() const noexcept;

private: // Attributes

  std::string                              m_Text;
  std::string                              m_Hint;
  std::function<void(const std::string &)> m_ChangeCallback;
};

} // namespace editor::ui
