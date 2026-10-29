#pragma once

#include "ui/DockedWindow.h"

#include <memory>

namespace editor
{

namespace core
{
class CLog;
} // namespace core

namespace ui
{
struct TToolBoxDesc;
} // namespace ui

class CConsoleWindow final:
  public ui::CDockedWindow
{
public: // Constructor

  CConsoleWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<core::CLog> _Log);

private: // CWindow

  void OnGui() override;

private: // Attributes

  std::shared_ptr<core::CLog> m_Log;
};

} // namespace editor
