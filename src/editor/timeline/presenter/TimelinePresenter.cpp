#include "TimelinePresenter.h"
#include "ITimelineTrackSource.h"
#include "TimelineSelectMode.h"
#include "TimelineSelection.h"
#include "core/ActionQueue.h"
#include "editor/timeline/TimelineDocument.h"
#include "editor/timeline/playback/TimelineKeyEvaluation.h"
#include "editor/timeline/playback/TimelinePlayer.h"

#include <algorithm>
#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CTimelinePresenter::CTimelinePresenter(std::shared_ptr<CTimelineDocument> _Document, std::shared_ptr<ITimelineEvaluator> _Evaluator, std::shared_ptr<const ITimelineTrackSource> _TrackSource, std::shared_ptr<core::CActionQueue> _ActionQueue):
  m_Document(std::move(_Document)),
  m_Player(std::make_unique<CTimelinePlayer>(std::move(_Evaluator))),
  m_TrackSource(std::move(_TrackSource)),
  m_Selection(std::make_shared<CTimelineSelection>()),
  m_ActionQueue(std::move(_ActionQueue)),
  m_ChangedConnection(0)
{
  m_ChangedConnection = m_Document->GetChangedSignal().Connect([this](const TTimelineChange & _Change)
  {
    OnDocumentChanged(_Change);
  });
}

CTimelinePresenter::~CTimelinePresenter()
{
  m_Document->GetChangedSignal().Disconnect(m_ChangedConnection);
}

//
// ITimelinePresenter
//

const CTimeline & CTimelinePresenter::GetTimeline() const
{
  return m_Document->GetTimeline();
}

const CTimelineSelection & CTimelinePresenter::GetSelection() const
{
  return *m_Selection;
}

TTimelineTicks CTimelinePresenter::GetTime() const
{
  return m_Player->GetTime();
}

bool CTimelinePresenter::IsPlaying() const
{
  return m_Player->IsPlaying();
}

bool CTimelinePresenter::IsPreviewing() const
{
  return m_Player->IsPreviewing();
}

bool CTimelinePresenter::IsLooping() const
{
  return m_Player->IsLooping();
}

bool CTimelinePresenter::CanAddKey(TTimelineId _TrackId) const
{
  const auto & Timeline = m_Document->GetTimeline();
  if (!Timeline.HasTrack(_TrackId))
    return false;

  const auto & Track = Timeline.GetTrack(_TrackId);
  if (Track.TypeId == TimelineGroupType || !Track.Children.empty())
    return false;

  return !std::holds_alternative<std::monostate>(FindKeyValue(Track, m_Player->GetTime()));
}

std::vector<TTimelineTrackTemplate> CTimelinePresenter::GetTrackTemplates() const
{
  return m_TrackSource->GetTrackTemplates(m_Document->GetTimeline());
}

void CTimelinePresenter::Play()
{
  m_Player->Play();
}

void CTimelinePresenter::Pause()
{
  m_Player->Pause();
}

void CTimelinePresenter::Stop()
{
  m_Player->Stop();
}

void CTimelinePresenter::SetLooping(bool _IsLooping)
{
  m_Player->SetLooping(_IsLooping);
}

void CTimelinePresenter::Scrub(TTimelineTicks _Time)
{
  m_Player->Scrub(_Time);
}

void CTimelinePresenter::SelectItem(TTimelineId _ItemId, ETimelineSelectMode _Mode)
{
  if (_Mode == ETimelineSelectMode::Toggle)
  {
    m_Selection->ToggleItem(_ItemId);
    return;
  }

  m_Selection->SelectItem(_ItemId, _Mode == ETimelineSelectMode::Add);
}

void CTimelinePresenter::SelectItems(const std::vector<TTimelineId> & _ItemIds, bool _IsAdding)
{
  if (!_IsAdding)
    m_Selection->ClearItems();

  for (const auto ItemId : _ItemIds)
    m_Selection->SelectItem(ItemId, true);
}

void CTimelinePresenter::SelectTrack(TTimelineId _TrackId)
{
  m_Selection->ClearItems();
  m_Selection->SelectTrack(_TrackId);
}

void CTimelinePresenter::PreviewMoveSelection(TTimelineTicks _Delta)
{
  m_ActionQueue->Post([Document = m_Document, ItemIds = GetSelectedItems(), _Delta]
  {
    Document->PreviewMoveItems(ItemIds, _Delta);
  });
}

void CTimelinePresenter::CommitMove()
{
  m_ActionQueue->Post([Document = m_Document]
  {
    Document->CommitPreview();
  });
}

void CTimelinePresenter::DeleteSelection()
{
  if (m_Selection->GetItems().empty())
  {
    if (m_Selection->GetTrack() != InvalidTimelineId)
      DeleteTrack(m_Selection->GetTrack());

    return;
  }

  m_ActionQueue->Post([Document = m_Document, ItemIds = GetSelectedItems()]
  {
    Document->RemoveItems(ItemIds);
  });
}

void CTimelinePresenter::AddKey(TTimelineId _TrackId, TTimelineTicks _Time)
{
  if (!CanAddKey(_TrackId))
    return;

  const auto & Track = m_Document->GetTimeline().GetTrack(_TrackId);
  const auto & Keys  = Track.Keys;

  TTimelineKey Key;
  Key.Time  = std::max<TTimelineTicks>(_Time, 0);
  Key.Value = FindKeyValue(Track, Key.Time);

  const auto Previous = std::find_if(Keys.rbegin(), Keys.rend(), [&Key](const TTimelineKey & _Existing)
  {
    return _Existing.Time <= Key.Time;
  });

  if (Previous != Keys.rend())
    Key.Interpolation = Previous->Interpolation;
  else
  if (!Keys.empty())
    Key.Interpolation = Keys.front().Interpolation;

  m_ActionQueue->Post([Document = m_Document, Selection = m_Selection, _TrackId, Key]
  {
    const auto KeyId = Document->AddKey(_TrackId, Key);
    if (KeyId != InvalidTimelineId)
      Selection->SelectItem(KeyId, false);
  });
}

void CTimelinePresenter::AddTrack(const TTimelineTrack & _Track)
{
  m_ActionQueue->Post([Document = m_Document, Selection = m_Selection, Location = FindNewTrackLocation(), _Track]
  {
    const auto TrackId = Document->AddTrack(Location, _Track);
    if (TrackId == InvalidTimelineId)
      return;

    Selection->ClearItems();
    Selection->SelectTrack(TrackId);
  });
}

void CTimelinePresenter::DeleteTrack(TTimelineId _TrackId)
{
  m_ActionQueue->Post([Document = m_Document, _TrackId]
  {
    Document->RemoveTrack(_TrackId);
  });
}

void CTimelinePresenter::RenameTrack(TTimelineId _TrackId, const std::string & _Name)
{
  m_ActionQueue->Post([Document = m_Document, _TrackId, _Name]
  {
    Document->RenameTrack(_TrackId, _Name);
  });
}

//
// Interface
//

void CTimelinePresenter::Tick(float _DeltaTime)
{
  m_Player->Update(m_Document->GetTimeline(), _DeltaTime);
}

//
// Implementation
//

std::vector<TTimelineId> CTimelinePresenter::GetSelectedItems() const
{
  const auto & Items = m_Selection->GetItems();
  return {Items.begin(), Items.end()};
}

TPropertyValue CTimelinePresenter::FindKeyValue(const TTimelineTrack & _Track, TTimelineTicks _Time) const
{
  if (!_Track.Keys.empty())
    return EvaluateKeys(_Track.Keys, _Time);

  return m_TrackSource->CaptureValue(_Track);
}

TTimelineTrackLocation CTimelinePresenter::FindNewTrackLocation() const
{
  const auto & Timeline = m_Document->GetTimeline();
  const auto   TrackId  = m_Selection->GetTrack();
  const auto   Location = Timeline.FindTrackLocation(TrackId);

  if (!Location)
    return {InvalidTimelineId, Timeline.GetTracks().size()};

  const auto & Track = Timeline.GetTrack(TrackId);
  if (Track.TypeId == TimelineGroupType)
    return {TrackId, Track.Children.size()};

  return {Location->ParentId, Location->Index + 1};
}

//
// Events
//

void CTimelinePresenter::OnDocumentChanged(const TTimelineChange & /*_Change*/)
{
  m_Selection->RemoveMissing(m_Document->GetTimeline());
}

} // namespace editor
