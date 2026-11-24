#pragma once

#include "Window.h"

#include <string>

namespace editor::ui
{

class CDockedWindow:
  public CWindow
{
protected: // Constructor

  CDockedWindow(std::string _Title, const std::string & _StableId, bool _Visible = true);

public: // CWindow

  bool IsOpen() const noexcept override;

protected: // CWindow

  bool BeginWindow() override;

  void EndWindow(bool _IsBegun) override;

public: // Interface

  bool IsVisible() const noexcept;

  void SetVisible(bool _Visible) noexcept;

  void SetTitleBarHidden(bool _IsTitleBarHidden) noexcept;

private: // Attributes

  bool m_Visible;
  bool m_IsTitleBarHidden;
};

} // namespace editor::ui
