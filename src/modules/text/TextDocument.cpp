#include "TextDocument.h"
#include "TextChange.h"
#include "TextEditCommand.h"
#include "core/WriteFileAtomic.h"
#include "editor/assets/AssetRecord.h"
#include "editor/documents/AssetProperty.h"

#include <algorithm>
#include <memory>
#include <utility>

namespace editor
{

//
// Constructor
//

CTextDocument::CTextDocument(TAssetRecord _Record, std::string _Content):
  CAssetDocument(std::move(_Record), "Text"),
  m_Content(std::move(_Content)),
  m_Revision(0),
  m_LastChangeEnd(0)
{
  // Empty
}

//
// CAssetDocument
//

std::vector<TAssetProperty> CTextDocument::GetProperties() const
{
  const auto LineCount = 1 + std::count(m_Content.begin(), m_Content.end(), '\n');

  return {{"Bytes", std::to_string(m_Content.size())},
          {"Lines", std::to_string(LineCount)       }};
}

//
// Protected CAssetDocument
//

bool CTextDocument::OnSave(std::string & _Error)
{
  return core::WriteFileAtomic(GetRecord().AbsolutePath, m_Content, _Error);
}

//
// Interface
//

const std::string & CTextDocument::GetContent() const noexcept
{
  return m_Content;
}

std::uint64_t CTextDocument::GetRevision() const noexcept
{
  return m_Revision;
}

std::size_t CTextDocument::GetLastChangeEnd() const noexcept
{
  return m_LastChangeEnd;
}

void CTextDocument::Edit(TTextChange _Change, unsigned int _EditSession)
{
  GetHistory().Execute(std::make_unique<CTextEditCommand>(*this, std::move(_Change), _EditSession));
}

void CTextDocument::Replace(std::size_t _Offset, std::size_t _Count, const std::string & _Text)
{
  const std::size_t Offset = std::min(_Offset, m_Content.size());

  m_Content.replace(Offset, _Count, _Text);

  m_LastChangeEnd = Offset + _Text.size();
  ++m_Revision;
}

} // namespace editor
