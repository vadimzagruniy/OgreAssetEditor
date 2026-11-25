#pragma once

#include <string>

namespace editor::ui
{

class CWindow
{
public: // Destructor

  virtual ~CWindow();

protected: // Constructor

  CWindow(std::string _Title, std::string _Label);

public: // Interface

  void Draw();

  virtual bool IsOpen() const noexcept = 0;

  const std::string & GetTitle() const noexcept;

  const std::string & GetLabel() const noexcept;

  int GetFocusFrame() const noexcept;

protected: // Derived interface

  virtual bool BeginWindow() = 0;

  virtual void EndWindow(bool _IsBegun) = 0;

protected: // Events

  virtual void OnGui() = 0;

private: // Attributes

  std::string m_Title;
  std::string m_Label;
  int         m_FocusFrame;
};

} // namespace editor::ui
