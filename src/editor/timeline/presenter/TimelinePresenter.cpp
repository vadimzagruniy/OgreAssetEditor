#include "TimelinePresenter.h"
#include "ITimelineTrackSource.h"
#include "TimelineSelectMode.h"
#include "TimelineSelection.h"
#include "core/ActionQueue.h"
#include "editor/timeline/TimelineDocument.h"
#include "editor/timeline/inspector/TimelineKeyPropertySource.h"
#include "editor/timeline/playback/TimelineKeyEvaluation.h"
#include "editor/timeline/playback/TimelinePlayer.h"
#include "editor/timeline/tracks/TimelineItemKind.h"
#include "editor/timeline/tracks/TimelineTrackEvaluation.h"
#include "editor/timeline/tracks/TimelineTrackTypes.h"
#include "editor/timeline/views/TimelineTime.h"

#include <algorithm>
#include <utility>

namespace editor
{

namespace
{

TTimelineId FindGroupBySlot(const std::vector<TTimelineTrack> & _Tracks, const std::string & _Slot)
{
  for (const auto & Track : _Tracks)
  {
    if (Track.TypeId == TimelineGroupType && Track.Target.Slot == _Slot)
      return Track.Id;

    if (const auto ChildId = FindGroupBySlot(Track.Children, _Slot); ChildId != InvalidTimelineId)
      return ChildId;
  }

  return InvalidTimelineId;
}

void AppendSelectedKeys(const std::vector<TTimelineTrack> & _Tracks, const CTimelineSelection & _Selection, std::vector<TTimelineClipboardKey> & _Keys)
{
  for (const auto & Track : _Tracks)
  {
    for (const auto & Key : Track.Keys)
    {
      if (_Selection.IsSelected(Key.Id))
        _Keys.push_back({Track.Id, Track.TypeId, Track.Target.Property, Key});
    }

    AppendSelectedKeys(Track.Children, _Selection, _Keys);
  }
}

} // namespace

//
// Constructor/Destructor
//

CTimelinePresenter::CTimelinePresenter(std::shared_ptr<CTimelineDocument> _Document, std::shared_ptr<const CTimelineTrackTypes> _TrackTypes, std::shared_ptr<const ITimelineTrackSource> _TrackSource, std::shared_ptr<core::CActionQueue> _ActionQueue):
  m_Document(std::move(_Document)),
  m_TrackTypes(std::move(_TrackTypes)),
  m_Player(std::make_unique<CTimelinePlayer>(std::make_shared<CTimelineTrackEvaluation>(m_TrackTypes))),
  m_TrackSource(std::move(_TrackSource)),
  m_Selection(std::make_shared<CTimelineSelection>()),
  m_KeyProperties(std::make_shared<CTimelineKeyPropertySource>(m_Document, m_Selection)),
  m_ActionQueue(std::move(_ActionQueue)),
  m_ChangedConnection(0),
  m_IsTrackTargetRequested(false),
  m_IsAddTrackMenuRequested(false),
  m_Clipboard()
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

float CTimelinePresenter::GetPlaybackSpeed() const
{
  return m_Player->GetSpeed();
}

ETimelineItemKind CTimelinePresenter::GetItemKind(const TTimelineTrack & _Track) const
{
  return m_TrackTypes->GetItemKind(_Track.TypeId);
}

bool CTimelinePresenter::CanAddKey(TTimelineId _TrackId) const
{
  const auto & Timeline = m_Document->GetTimeline();
  if (!Timeline.HasTrack(_TrackId))
    return false;

  const auto & Track = Timeline.GetTrack(_TrackId);
  if (GetItemKind(Track) != ETimelineItemKind::Keys)
    return false;

  return !std::holds_alternative<std::monostate>(FindKeyValue(Track, m_Player->GetTime()));
}

bool CTimelinePresenter::IsTargetValid(const TTimelineTrack & _Track) const
{
  return m_TrackSource->IsTargetValid(_Track);
}

bool CTimelinePresenter::CanBindToSelection(const TTimelineTrack & _Track) const
{
  return m_TrackSource->CanBindToSelection(_Track);
}

std::uint32_t CTimelinePresenter::GetTrackColor(const TTimelineTrack & _Track) const
{
  return m_TrackSource->GetTrackColor(_Track);
}

std::vector<TTimelineTrackTemplate> CTimelinePresenter::GetTrackTemplates() const
{
  return m_TrackSource->GetTrackTemplates(m_Document->GetTimeline());
}

std::vector<TTimelineTrackTemplate> CTimelinePresenter::GetTargetTemplates(const TTimelineTrack & _Target) const
{
  return m_TrackSource->GetTargetTemplates(m_Document->GetTimeline(), _Target.Target.Slot);
}

std::string CTimelinePresenter::GetTrackLabel(const TTimelineTrack & _Track) const
{
  return m_TrackSource->GetTrackLabel(_Track);
}

bool CTimelinePresenter::HasTrackTarget() const
{
  return m_TrackSource->HasTrackTarget();
}

bool CTimelinePresenter::IsTrackTargetRequested() const
{
  return m_IsTrackTargetRequested;
}

bool CTimelinePresenter::IsAddTrackMenuRequested() const
{
  return m_IsAddTrackMenuRequested;
}

bool CTimelinePresenter::CanPasteKeys() const
{
  return !m_Clipboard.empty();
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

void CTimelinePresenter::SetPlaybackSpeed(float _Speed)
{
  m_Player->SetSpeed(_Speed);
}

void CTimelinePresenter::Scrub(TTimelineTicks _Time)
{
  m_Player->Scrub(_Time);
}

void CTimelinePresenter::SetDuration(TTimelineTicks _Duration)
{
  m_ActionQueue->Post([Document = m_Document, _Duration]
  {
    Document->SetDuration(_Duration);
  });
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

void CTimelinePresenter::CopySelection()
{
  TTimelineTicks StartTime = 0;
  auto           Keys      = CollectSelectedKeys(StartTime);

  if (!Keys.empty())
    m_Clipboard = std::move(Keys);
}

void CTimelinePresenter::PasteKeys(TTimelineTicks _Time)
{
  const bool IsSingleTrack = std::all_of(m_Clipboard.begin(), m_Clipboard.end(), [this](const TTimelineClipboardKey & _Entry)
  {
    return _Entry.TrackId == m_Clipboard.front().TrackId;
  });

  std::vector<TTimelineTrackKey> Keys;

  for (const auto & Entry : m_Clipboard)
  {
    const TTimelineId TrackId = FindPasteTrack(Entry, IsSingleTrack);
    if (TrackId == InvalidTimelineId)
      continue;

    TTimelineKey Key = Entry.Key;
    Key.Time += _Time;

    Keys.push_back({TrackId, std::move(Key)});
  }

  InsertKeys("Paste Keys", std::move(Keys));
}

void CTimelinePresenter::DuplicateSelection()
{
  TTimelineTicks StartTime = 0;
  const auto     Selected  = CollectSelectedKeys(StartTime);

  if (Selected.empty())
    return;

  const auto LastKey = std::max_element(Selected.begin(), Selected.end(), [](const TTimelineClipboardKey & _Left, const TTimelineClipboardKey & _Right)
  {
    return _Left.Key.Time < _Right.Key.Time;
  });

  const TTimelineTicks Offset = StartTime + LastKey->Key.Time + GetFrameTicks(m_Document->GetTimeline().GetFrameRate());

  std::vector<TTimelineTrackKey> Keys;

  for (const auto & Entry : Selected)
  {
    TTimelineKey Key = Entry.Key;
    Key.Time += Offset;

    Keys.push_back({Entry.TrackId, std::move(Key)});
  }

  InsertKeys("Duplicate Keys", std::move(Keys));
}

void CTimelinePresenter::SetSelectionInterpolation(ETimelineInterpolation _Interpolation)
{
  m_ActionQueue->Post([Document = m_Document, ItemIds = GetSelectedItems(), _Interpolation]
  {
    Document->SetKeysInterpolation(ItemIds, _Interpolation);
  });
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
  const auto &   Timeline   = m_Document->GetTimeline();
  const bool     IsTargeted = _Track.TypeId != TimelineGroupType && !_Track.Target.Slot.empty();
  const auto     GroupId    = IsTargeted ? FindTargetGroup(_Track.Target.Slot) : InvalidTimelineId;
  const bool     IsWrapped  = IsTargeted && GroupId == InvalidTimelineId;
  TTimelineTrack Track      = _Track;
  auto           Location   = FindNewTrackLocation();

  if (GroupId != InvalidTimelineId)
    Location = {GroupId, Timeline.GetTrack(GroupId).Children.size()};

  if (IsWrapped)
  {
    TTimelineTrack Group;
    Group.TypeId = TimelineGroupType;
    Group.Target = {_Track.Target.Slot, {}};
    Group.Name   = m_TrackSource->GetTrackLabel(Group);
    Group.Children.push_back(std::move(Track));

    Track    = std::move(Group);
    Location = {InvalidTimelineId, Timeline.GetTracks().size()};
  }

  m_ActionQueue->Post([Document = m_Document, Selection = m_Selection, TrackSource = m_TrackSource, Location, Track = std::move(Track), Added = _Track, IsWrapped]
  {
    const auto TrackId = Document->AddTrack(Location, Track);
    if (TrackId == InvalidTimelineId)
      return;

    TrackSource->OnTrackAdded(Added);

    Selection->ClearItems();
    Selection->SelectTrack(IsWrapped ? Document->GetTimeline().GetTrack(TrackId).Children.front().Id : TrackId);
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

void CTimelinePresenter::BindToSelection(const TTimelineTrack & _Track)
{
  m_TrackSource->BindToSelection(_Track);
}

void CTimelinePresenter::ShowTrackInScene(TTimelineId _TrackId)
{
  m_ActionQueue->Post([Document = m_Document, TrackSource = m_TrackSource, _TrackId]
  {
    if (Document->GetTimeline().HasTrack(_TrackId))
      TrackSource->ShowTrackInScene(Document->GetTimeline().GetTrack(_TrackId));
  });
}

void CTimelinePresenter::RequestTrackTarget()
{
  m_IsTrackTargetRequested = true;
}

void CTimelinePresenter::ConfirmTrackTarget(const std::string & _Target)
{
  m_IsTrackTargetRequested  = false;
  m_IsAddTrackMenuRequested = true;

  m_ActionQueue->Post([TrackSource = m_TrackSource, _Target]
  {
    TrackSource->SelectTrackTarget(_Target);
  });
}

void CTimelinePresenter::CancelTrackTarget()
{
  m_IsTrackTargetRequested = false;
}

void CTimelinePresenter::ClearAddTrackMenuRequest()
{
  m_IsAddTrackMenuRequested = false;
}

//
// Interface
//

void CTimelinePresenter::Tick(float _DeltaTime)
{
  m_Player->Update(m_Document->GetTimeline(), _DeltaTime);
}

std::shared_ptr<IPropertySource> CTimelinePresenter::GetInspected() const
{
  return m_KeyProperties->HasKey() ? m_KeyProperties : nullptr;
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
  if (GetItemKind(Track) == ETimelineItemKind::Group)
    return {TrackId, Track.Children.size()};

  return {Location->ParentId, Location->Index + 1};
}

TTimelineId CTimelinePresenter::FindTargetGroup(const std::string & _Slot) const
{
  return FindGroupBySlot(m_Document->GetTimeline().GetTracks(), _Slot);
}

std::vector<TTimelineClipboardKey> CTimelinePresenter::CollectSelectedKeys(TTimelineTicks & _StartTime) const
{
  std::vector<TTimelineClipboardKey> Keys;
  AppendSelectedKeys(m_Document->GetTimeline().GetTracks(), *m_Selection, Keys);

  if (Keys.empty())
    return Keys;

  const auto FirstKey = std::min_element(Keys.begin(), Keys.end(), [](const TTimelineClipboardKey & _Left, const TTimelineClipboardKey & _Right)
  {
    return _Left.Key.Time < _Right.Key.Time;
  });

  _StartTime = FirstKey->Key.Time;

  for (auto & Entry : Keys)
    Entry.Key.Time -= _StartTime;

  return Keys;
}

TTimelineId CTimelinePresenter::FindPasteTrack(const TTimelineClipboardKey & _Entry, bool _IsSingleTrack) const
{
  const auto &      Timeline      = m_Document->GetTimeline();
  const TTimelineId SelectedTrack = m_Selection->GetTrack();

  if (_IsSingleTrack && Timeline.HasTrack(SelectedTrack))
  {
    const auto & Track = Timeline.GetTrack(SelectedTrack);

    if (Track.TypeId == _Entry.TypeId && Track.Target.Property == _Entry.Property && GetItemKind(Track) == ETimelineItemKind::Keys)
      return SelectedTrack;
  }

  return Timeline.HasTrack(_Entry.TrackId) ? _Entry.TrackId : InvalidTimelineId;
}

void CTimelinePresenter::InsertKeys(std::string _CommandName, std::vector<TTimelineTrackKey> _Keys)
{
  if (_Keys.empty())
    return;

  m_ActionQueue->Post([Document = m_Document, Selection = m_Selection, CommandName = std::move(_CommandName), Keys = std::move(_Keys)]
  {
    const auto KeyIds = Document->AddKeys(CommandName, Keys);
    if (KeyIds.empty())
      return;

    Selection->ClearItems();

    for (const auto KeyId : KeyIds)
      Selection->SelectItem(KeyId, true);
  });
}

//
// Events
//

void CTimelinePresenter::OnDocumentChanged(const TTimelineChange & /*_Change*/)
{
  m_Selection->RemoveMissing(m_Document->GetTimeline());
}

} // namespace editor
