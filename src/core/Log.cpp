#include "Log.h"

#include <utility>

namespace editor::core
{

//
// Interface
//

void CLog::Write(ELogLevel _Level, std::string _Message)
{
  m_Entries.push_back({_Level, std::move(_Message)});
}

const std::vector<TLogEntry> & CLog::GetEntries() const noexcept
{
  return m_Entries;
}

void CLog::Clear()
{
  m_Entries.clear();
}

} // namespace editor::core
