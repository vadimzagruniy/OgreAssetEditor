#pragma once

#include "LogEntry.h"

#include <string>
#include <vector>

namespace editor::core
{

class CLog
{
public: // Interface

  void Write(ELogLevel _Level, std::string _Message);

  const std::vector<TLogEntry> & GetEntries() const noexcept;

  void Clear();

private: // Attributes

  std::vector<TLogEntry> m_Entries;
};

} // namespace editor::core
