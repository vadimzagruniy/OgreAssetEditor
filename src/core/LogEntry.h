#pragma once

#include <string>

namespace editor::core
{

enum class ELogLevel
{
  Info,
  Warning,
  Error
};

struct TLogEntry
{
  ELogLevel   Level;
  std::string Message;
};

} // namespace editor::core
