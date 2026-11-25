#include "EditorWorkspace.h"
#include "IAttachedEditor.h"
#include "core/EditorService.h"
#include "editor/assets/AssetRecord.h"
#include "editor/documents/AssetDocument.h"

#include <algorithm>
#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CEditorWorkspace::CEditorWorkspace(std::shared_ptr<CAssetDocument> _Document):
  m_Document(std::move(_Document)),
  m_Services(std::make_unique<core::CEditorService>()),
  m_FocusFrame(-1),
  m_IsActive(false)
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
  if (m_Document->IsDirty())
    return true;

  return std::any_of(m_AttachedEditors.begin(), m_AttachedEditors.end(), [](const std::shared_ptr<IAttachedEditor> & _Editor)
  {
    return _Editor->GetDocument().IsDirty();
  });
}

bool CEditorWorkspace::Save(std::string & _Error)
{
  bool IsSaved = !m_Document->IsDirty() || m_Document->Save(_Error);

  for (const auto & Editor : m_AttachedEditors)
  {
    auto & Document = Editor->GetDocument();

    if (!Document.IsDirty())
      continue;

    std::string Error;
    if (Document.Save(Error))
      continue;

    if (!_Error.empty())
      _Error += "\n";

    _Error += Error.empty() ? "Cannot save " + Document.GetRecord().RelativePath.filename().string() : Error;
    IsSaved = false;
  }

  return IsSaved;
}

void CEditorWorkspace::Activate()
{
  m_IsActive = true;

  OnActivated();

  for (const auto & Editor : m_AttachedEditors)
    Editor->OnActivated();
}

void CEditorWorkspace::Deactivate()
{
  for (const auto & Editor : m_AttachedEditors)
    Editor->OnDeactivated();

  OnDeactivated();

  m_IsActive = false;
}

bool CEditorWorkspace::IsActive() const noexcept
{
  return m_IsActive;
}

void CEditorWorkspace::Tick(float _DeltaTime)
{
  OnTick(_DeltaTime);

  for (const auto & Editor : m_AttachedEditors)
    Editor->Tick(_DeltaTime);
}

core::CEditorService & CEditorWorkspace::GetServices() const noexcept
{
  return *m_Services;
}

void CEditorWorkspace::AttachEditor(std::shared_ptr<IAttachedEditor> _Editor)
{
  m_AttachedEditors.push_back(std::move(_Editor));

  if (m_IsActive)
    m_AttachedEditors.back()->OnActivated();
}

void CEditorWorkspace::DetachEditor(const IAttachedEditor & _Editor)
{
  const auto EditorIterator = std::find_if(m_AttachedEditors.begin(), m_AttachedEditors.end(), [&_Editor](const std::shared_ptr<IAttachedEditor> & _Attached)
  {
    return _Attached.get() == &_Editor;
  });

  if (EditorIterator == m_AttachedEditors.end())
    return;

  if (m_IsActive)
    (*EditorIterator)->OnDeactivated();

  m_AttachedEditors.erase(EditorIterator);
}

const std::vector<std::shared_ptr<IAttachedEditor>> & CEditorWorkspace::GetAttachedEditors() const noexcept
{
  return m_AttachedEditors;
}

int CEditorWorkspace::GetFocusFrame() const noexcept
{
  return m_FocusFrame;
}

void CEditorWorkspace::SetFocusFrame(int _Frame) noexcept
{
  m_FocusFrame = _Frame;
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

void CEditorWorkspace::OnTick(float /*_DeltaTime*/)
{
  // Empty
}

} // namespace editor
