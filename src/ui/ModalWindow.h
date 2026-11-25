#pragma once

#include "ModalButton.h"
#include "Window.h"

#include <imgui.h>

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace editor::ui
{

class CModalWindow:
  public CWindow
{
protected: // Constructor

  CModalWindow(const std::string & _Title, const ImVec2 & _Size);

public: // CWindow

  bool IsOpen() const noexcept override;

protected: // CWindow

  bool BeginWindow() override;

  void EndWindow(bool _IsBegun) override;

  void OnGui() final;

protected: // Interface

  std::size_t AddButton(std::string _Label, std::function<void()> _Action);

  void SetButtonEnabled(std::size_t _ButtonIndex, bool _IsEnabled);

  void Close() noexcept;

private: // Implementation

  void DrawButtons();

  void Cancel();

protected: // Events

  virtual void OnContentGui() = 0;

  virtual void OnCancel();

private: // Attributes

  ImVec2                    m_Size;
  std::vector<TModalButton> m_Buttons;
  bool                      m_IsOpened;
  bool                      m_IsClosed;
};

} // namespace editor::ui
