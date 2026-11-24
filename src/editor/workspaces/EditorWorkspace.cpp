#include "EditorWorkspace.h"
#include "editor/assets/AssetRecord.h"
#include "editor/documents/AssetDocument.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CEditorWorkspace::CEditorWorkspace(std::shared_ptr<CAssetDocument> _Document):
  m_Document(std::move(_Document))
{
  // Empty
}

CEditorWorkspace::~CEditorWorkspace() = default;

//
// Interface
//

CAssetDocument & CEditorWorkspace::GetDocument() const noexcept
{
  return *m_Document;
}

std::string CEditorWorkspace::GetTitle() const
{
  return m_Document->GetRecord().RelativePath.filename().string();
}

bool CEditorWorkspace::IsDirty() const
{
  return m_Document->IsDirty();
}

bool CEditorWorkspace::Save(std::string & _Error)
{
  OnBeforeSave();

  return m_Document->Save(_Error);
}

std::vector<core::TTypeKey> CEditorWorkspace::GetToolBoxes() const
{
  return {};
}

std::shared_ptr<IPropertySource> CEditorWorkspace::GetInspected() const
{
  return nullptr;
}

//
// Events
//

void CEditorWorkspace::OnActivated()
{
  // Empty
}

void CEditorWorkspace::OnDeactivated()
{
  // Empty
}

void CEditorWorkspace::OnBeforeSave()
{
  // Empty
}

} // namespace editor
