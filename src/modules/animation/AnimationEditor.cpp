#include "AnimationEditor.h"
#include "AnimationDocument.h"
#include "AnimationPreview.h"
#include "editor/timeline/playback/TimelinePlayer.h"
#include "editor/timeline/views/TimelineView.h"
#include "views/AnimationTimelineWindow.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CAnimationEditor::CAnimationEditor(std::shared_ptr<CAnimationDocument> _Document, std::weak_ptr<ISceneAnimationHost> _Host):
  m_Document(std::move(_Document)),
  m_TimelineView(std::make_unique<CTimelineView>()),
  m_Player(std::make_unique<CTimelinePlayer>(std::make_shared<CAnimationPreview>(std::move(_Host))))
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

void CAnimationEditor::Tick(float _DeltaTime)
{
  m_Player->Update(m_Document->GetTimeline(), _DeltaTime);
}

void CAnimationEditor::OnActivated()
{
  // Empty
}

void CAnimationEditor::OnDeactivated()
{
  m_Player->Stop();
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

CTimelinePlayer & CAnimationEditor::GetPlayer() const noexcept
{
  return *m_Player;
}

} // namespace editor
