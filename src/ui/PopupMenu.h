#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace editor::ui
{

class CPopupMenu
{
public: // Constructor

  explicit CPopupMenu(const std::string & _Identifier);

public: // Setup

  CPopupMenu & SetItems(std::vector<std::string> _Items);

  CPopupMenu & OnSelected(std::function<void(std::size_t)> _Callback);

public: // Interface

  void Open() noexcept;

  void Draw();

private: // Attributes

  std::string                      m_Label;
  std::vector<std::string>         m_Items;
  std::function<void(std::size_t)> m_SelectCallback;
  bool                             m_IsOpenRequested;
};

} // namespace editor::ui
