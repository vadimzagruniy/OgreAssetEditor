#pragma once

#include <string>

namespace editor::ui
{

class CWindow
{
public: // Destructor

  virtual ~CWindow();

public: // Interface

  void Draw();

  bool IsVisible() const noexcept;

  void SetVisible(bool _Visible) noexcept;

  void SetTitleBarHidden(bool _IsTitleBarHidden) noexcept;

  const std::string & GetTitle() const noexcept;

  const std::string & GetLabel() const noexcept;

protected: // Constructor

  CWindow(std::string _Title, std::string _StableId, bool _Visible = true);

protected: // Interface

  virtual void OnGui() = 0;

private: // Attributes

  std::string m_Title;
  std::string m_Label;
  bool        m_Visible;
  bool        m_IsTitleBarHidden;
};

} // namespace editor::ui
