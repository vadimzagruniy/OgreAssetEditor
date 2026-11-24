#pragma once

#include "Window.h"

#include <string>

namespace editor::ui
{

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

protected: // Interface

  void Close() noexcept;

private: // Implementation

  void Cancel();

protected: // Events

  virtual void OnCancel();

private: // Attributes

  bool m_IsOpened;
  bool m_IsClosed;
};

} // namespace editor::ui
