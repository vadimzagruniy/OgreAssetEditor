#pragma once

#include <string>

namespace editor::ui
{

class CModalWindow
{
public: // Destructor

  virtual ~CModalWindow();

public: // Interface

  bool Draw();

  const std::string & GetTitle() const noexcept;

protected: // Constructor

  explicit CModalWindow(std::string _Title);

protected: // Interface

  void Close() noexcept;

protected: // Events

  virtual void OnGui() = 0;

  virtual void OnCancel();

private: // Attributes

  std::string m_Title;
  std::string m_Label;
  bool        m_IsOpened;
  bool        m_IsClosed;
};

} // namespace editor::ui
