#include "TextInstance.h"
#include "editor/assets/AssetRecord.h"
#include "editor/workspaces/AssetProperty.h"

#include <algorithm>
#include <fstream>
#include <utility>

namespace editor
{

//
// Constructor
//

CTextInstance::CTextInstance(TAssetRecord _Record, std::string _Content):
  CAssetInstance(std::move(_Record), "Text"),
  m_Content(std::move(_Content)),
  m_Revision(0),
  m_LastChangeEnd(0)
{
  // Empty
}

//
// CAssetInstance
//

std::vector<TAssetProperty> CTextInstance::GetProperties() const
{
  const auto LineCount = 1 + std::count(m_Content.begin(), m_Content.end(), '\n');

  return {{"Bytes", std::to_string(m_Content.size())},
          {"Lines", std::to_string(LineCount)       }};
}

//
// Interface
//

const std::string & CTextInstance::GetContent() const noexcept
{
  return m_Content;
}

std::uint64_t CTextInstance::GetRevision() const noexcept
{
  return m_Revision;
}

std::size_t CTextInstance::GetLastChangeEnd() const noexcept
{
  return m_LastChangeEnd;
}

void CTextInstance::Replace(std::size_t _Offset, std::size_t _Count, const std::string & _Text)
{
  const std::size_t Offset = std::min(_Offset, m_Content.size());

  m_Content.replace(Offset, _Count, _Text);

  m_LastChangeEnd = Offset + _Text.size();
  ++m_Revision;
}

bool CTextInstance::Save()
{
  std::ofstream File(GetRecord().AbsolutePath, std::ios::binary | std::ios::trunc);
  File.write(m_Content.data(), static_cast<std::streamsize>(m_Content.size()));

  return static_cast<bool>(File);
}

} // namespace editor
