#include "AssetDocument.h"
#include "editor/assets/AssetRecord.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CAssetDocument::CAssetDocument(TAssetRecord _Record, std::string _TypeName):
  m_Record(std::make_unique<TAssetRecord>(std::move(_Record))),
  m_TypeName(std::move(_TypeName))
{
  // Empty
}

CAssetDocument::~CAssetDocument() = default;

//
// Interface
//

const TAssetRecord & CAssetDocument::GetRecord() const noexcept
{
  return *m_Record;
}

void CAssetDocument::SetRecord(TAssetRecord _Record)
{
  *m_Record = std::move(_Record);
}

const std::string & CAssetDocument::GetTypeName() const noexcept
{
  return m_TypeName;
}

core::CUndoStack & CAssetDocument::GetHistory() noexcept
{
  return m_History;
}

const core::CUndoStack & CAssetDocument::GetHistory() const noexcept
{
  return m_History;
}

bool CAssetDocument::IsDirty() const noexcept
{
  return !m_History.IsAtSavePoint();
}

bool CAssetDocument::Save(std::string & _Error)
{
  if (!OnSave(_Error))
    return false;

  m_History.MarkSaved();

  return true;
}

//
// Events
//

bool CAssetDocument::OnSave(std::string & /*_Error*/)
{
  return true;
}

} // namespace editor
