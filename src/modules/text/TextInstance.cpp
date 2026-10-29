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
  m_Content(std::move(_Content))
{
  m_Content.reserve(std::min(MaxContentBytes, m_Content.size() + 4096));
}

//
// Interface
//

std::string & CTextInstance::GetContent() noexcept
{
  return m_Content;
}

bool CTextInstance::IsDirty() const noexcept
{
  return m_Dirty;
}

void CTextInstance::MarkDirty() noexcept
{
  m_Dirty = true;
}

bool CTextInstance::Save()
{
  std::ofstream File(GetRecord().AbsolutePath, std::ios::binary | std::ios::trunc);
  File.write(m_Content.data(), static_cast<std::streamsize>(m_Content.size()));

  if (!File)
    return false;

  m_Dirty = false;

  return true;
}

//
// CAssetInstance
//

std::vector<TAssetProperty> CTextInstance::GetProperties() const
{
  const auto LineCount = 1 + std::count(m_Content.begin(), m_Content.end(), '\n');

  return {{"Bytes",    std::to_string(m_Content.size())},
          {"Lines",    std::to_string(LineCount)       },
          {"Modified", m_Dirty ? "Yes" : "No"          }};
}

} // namespace editor
