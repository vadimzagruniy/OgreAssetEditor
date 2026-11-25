#include "AnimationEditor.h"
#include "AnimationDocument.h"
#include "AnimationTrackSource.h"
#include "AnimationTracks.h"
#include "NodePropertyEvaluator.h"
#include "editor/timeline/presenter/TimelinePresenter.h"
#include "editor/timeline/tracks/TimelineTrackTypes.h"
#include "editor/timeline/views/TimelineView.h"
#include "views/AnimationTimelineWindow.h"

#include <utility>

namespace editor
{

namespace
{

std::shared_ptr<const CTimelineTrackTypes> CreateTrackTypes(const std::weak_ptr<ISceneAnimationHost> & _Host)
{
  auto TrackTypes = std::make_shared<CTimelineTrackTypes>();
  TrackTypes->Register({std::string(NodePropertyTrackType), "Node Property", ETimelineItemKind::Keys}, std::make_shared<CNodePropertyEvaluator>(_Host));

  return TrackTypes;
}

} // namespace

//
// Constructor/Destructor
//

CAnimationEditor::CAnimationEditor(std::shared_ptr<CAnimationDocument> _Document, std::weak_ptr<ISceneAnimationHost> _Host, std::shared_ptr<core::CActionQueue> _ActionQueue):
  m_Document(std::move(_Document)),
  m_TimelineView(std::make_unique<CTimelineView>()),
  m_Presenter(std::make_unique<CTimelinePresenter>(m_Document, CreateTrackTypes(_Host), std::make_shared<CAnimationTrackSource>(_Host), std::move(_ActionQueue)))
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

std::shared_ptr<IPropertySource> CAnimationEditor::GetInspected() const
{
  return m_Presenter->GetInspected();
}

void CAnimationEditor::Tick(float _DeltaTime)
{
  m_Presenter->Tick(_DeltaTime);
}

void CAnimationEditor::OnActivated()
{
  // Empty
}

void CAnimationEditor::OnDeactivated()
{
  m_Presenter->Stop();
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

CTimelinePresenter & CAnimationEditor::GetPresenter() const noexcept
{
  return *m_Presenter;
}

} // namespace editor
