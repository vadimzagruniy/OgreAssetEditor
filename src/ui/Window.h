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

  const std::string & GetTitle() const noexcept;

protected: // Constructor

  CWindow(std::string _Title, std::string _StableId, bool _Visible = true);

protected: // Interface

  virtual void OnGui() = 0;

private: // Attributes

  std::string m_Title;
  std::string m_StableId;
  bool        m_Visible = true;
};

} // namespace editor::ui
