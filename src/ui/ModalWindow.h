#pragma once

#include "Window.h"
#include "ui/layout/Layout.h"

#include <functional>
#include <string>

namespace editor::ui
{

class CButton;

class CModalWindow:
  public CWindow
{
protected: // Constructor

  explicit CModalWindow(const std::string & _Title);

public: // CWindow

  bool IsOpen() const noexcept override;

protected: // CWindow

  bool BeginWindow() override;

  void EndWindow(bool _IsBegun) override;

  void OnGui() final;

protected: // Interface

  CButton & AddButton(std::string _Label, std::function<void()> _Action);

  void Close() noexcept;

private: // Implementation

  void Cancel();

protected: // Events

  virtual void OnContentGui() = 0;

  virtual void OnCancel();

private: // Attributes

  CLayout m_Buttons;
  bool    m_IsOpened;
  bool    m_IsClosed;
};

} // namespace editor::ui
