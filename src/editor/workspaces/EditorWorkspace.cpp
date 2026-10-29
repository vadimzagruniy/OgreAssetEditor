#include "EditorWorkspace.h"
#include "AssetInstance.h"
#include "editor/assets/AssetRecord.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CEditorWorkspace::CEditorWorkspace(std::unique_ptr<CAssetInstance> _Instance):
  m_Instance(std::move(_Instance))
{
  // Empty
}

CEditorWorkspace::~CEditorWorkspace() = default;

//
// Interface
//

CAssetInstance * CEditorWorkspace::GetInstance() const noexcept
{
  return m_Instance.get();
}

core::CUndoStack & CEditorWorkspace::GetUndoStack() noexcept
{
  return m_UndoStack;
}

const std::vector<std::string> & CEditorWorkspace::GetSelection() const noexcept
{
  return m_Selection;
}

void CEditorWorkspace::SetSelection(std::vector<std::string> _Selection)
{
  m_Selection = std::move(_Selection);
}

std::string CEditorWorkspace::GetTitle() const
{
  return m_Instance->GetRecord().RelativePath.filename().string();
}

//
// IWorkspaceContext
//

CEditorWorkspace * CEditorWorkspace::GetWorkspace() const noexcept
{
  return const_cast<CEditorWorkspace *>(this);
}

CAssetInstance * CEditorWorkspace::GetAssetInstance() const noexcept
{
  return m_Instance.get();
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

} // namespace editor
