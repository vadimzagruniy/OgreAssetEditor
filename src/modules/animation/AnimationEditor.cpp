#include "AnimationEditor.h"
#include "AnimationDocument.h"
#include "editor/timeline/views/TimelineView.h"
#include "views/AnimationTimelineWindow.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CAnimationEditor::CAnimationEditor(std::shared_ptr<CAnimationDocument> _Document):
  m_Document(std::move(_Document)),
  m_TimelineView(std::make_unique<CTimelineView>())
{
  // Empty
}

CAnimationEditor::~CAnimationEditor() = default;

//
// IAttachedEditor
//

CAssetDocument & CAnimationEditor::GetDocument() const noexcept
{
  return *m_Document;
}

std::vector<core::TTypeKey> CAnimationEditor::GetToolBoxes() const
{
  return {core::TypeOf<CAnimationTimelineWindow>()};
}

void CAnimationEditor::OnActivated()
{
  // Empty
}

void CAnimationEditor::OnDeactivated()
{
  m_Document->CommitPreview();
}

//
// Interface
//

CAnimationDocument & CAnimationEditor::GetAnimation() const noexcept
{
  return *m_Document;
}

CTimelineView & CAnimationEditor::GetTimelineView() const noexcept
{
  return *m_TimelineView;
}

} // namespace editor
