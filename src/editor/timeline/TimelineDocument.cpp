#include "TimelineDocument.h"
#include "TimelineChange.h"
#include "core/FunctionCommand.h"
#include "editor/assets/AssetRecord.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <unordered_set>
#include <utility>

namespace editor
{

namespace
{

void ExpectSuccess(bool _IsDone)
{
  assert(_IsDone && "Timeline command failed on a consistent undo stack");
  (void)_IsDone;
}

std::size_t GetChildCount(const CTimeline & _Timeline, TTimelineId _ParentId)
{
  return _ParentId == InvalidTimelineId ? _Timeline.GetTracks().size() : _Timeline.GetTrack(_ParentId).Children.size();
}

TTimelineTicks FindEarliestTime(const std::vector<TTimelineTrack> & _Tracks, const std::unordered_set<TTimelineId> & _ItemIds)
{
  TTimelineTicks Earliest = std::numeric_limits<TTimelineTicks>::max();

  for (const auto & Track : _Tracks)
  {
    for (const auto & Key : Track.Keys)
    {
      if (_ItemIds.contains(Key.Id))
        Earliest = std::min(Earliest, Key.Time);
    }

    for (const auto & Clip : Track.Clips)
    {
      if (_ItemIds.contains(Clip.Id))
        Earliest = std::min(Earliest, Clip.Start);
    }
  }

  return Earliest == std::numeric_limits<TTimelineTicks>::max() ? 0 : Earliest;
}

void ShiftItems(TTimelineTrack & _Track, const std::unordered_set<TTimelineId> & _ItemIds, TTimelineTicks _Delta)
{
  for (auto & Key : _Track.Keys)
  {
    if (_ItemIds.contains(Key.Id))
      Key.Time += _Delta;
  }

  for (auto & Clip : _Track.Clips)
  {
    if (_ItemIds.contains(Clip.Id))
      Clip.Start += _Delta;
  }
}

} // namespace

//
// Constructor/Destructor
//

CTimelineDocument::CTimelineDocument(TAssetRecord _Record, std::string _TypeName):
  CAssetDocument(std::move(_Record), std::move(_TypeName))
{
  // Empty
}

CTimelineDocument::~CTimelineDocument() = default;

//
// Interface
//

const CTimeline & CTimelineDocument::GetTimeline() const noexcept
{
  return m_Timeline;
}

void CTimelineDocument::SetDuration(TTimelineTicks _Duration)
{
  const TTimelineTicks OldDuration = m_Timeline.GetDuration();
  if (OldDuration == _Duration)
    return;

  auto Apply = [](TTimelineTicks _Value)
  {
    return [_Value](CTimelineDocument & _Document)
    {
      _Document.m_Timeline.SetDuration(_Value);
      _Document.Notify(ETimelineChangeKind::TimelineChanged);
    };
  };

  ExecuteCommand("Set Duration", Apply(_Duration), Apply(OldDuration));
}

void CTimelineDocument::SetFrameRate(int _FrameRate)
{
  const int OldFrameRate = m_Timeline.GetFrameRate();
  if (OldFrameRate == _FrameRate)
    return;

  auto Apply = [](int _Value)
  {
    return [_Value](CTimelineDocument & _Document)
    {
      _Document.m_Timeline.SetFrameRate(_Value);
      _Document.Notify(ETimelineChangeKind::TimelineChanged);
    };
  };

  ExecuteCommand("Set Frame Rate", Apply(_FrameRate), Apply(OldFrameRate));
}

TTimelineId CTimelineDocument::AddTrack(const TTimelineTrackLocation & _Location, TTimelineTrack _Track)
{
  if (_Location.ParentId != InvalidTimelineId && !m_Timeline.HasTrack(_Location.ParentId))
    return InvalidTimelineId;

  if (_Location.Index > GetChildCount(m_Timeline, _Location.ParentId))
    return InvalidTimelineId;

  AssignIds(_Track);

  const TTimelineId TrackId = _Track.Id;

  ExecuteCommand("Add Track",
    [_Location, Track = std::move(_Track)](CTimelineDocument & _Document)
    {
      ExpectSuccess(_Document.m_Timeline.InsertTrack(_Location, Track));
      _Document.Notify(ETimelineChangeKind::TrackAdded, Track.Id);
    },
    [TrackId](CTimelineDocument & _Document)
    {
      ExpectSuccess(_Document.m_Timeline.RemoveTrack(TrackId));
      _Document.Notify(ETimelineChangeKind::TrackRemoved, TrackId);
    });

  return TrackId;
}

void CTimelineDocument::RemoveTrack(TTimelineId _TrackId)
{
  const auto Location = m_Timeline.FindTrackLocation(_TrackId);
  if (!Location)
    return;

  ExecuteCommand("Delete Track",
    [_TrackId](CTimelineDocument & _Document)
    {
      ExpectSuccess(_Document.m_Timeline.RemoveTrack(_TrackId));
      _Document.Notify(ETimelineChangeKind::TrackRemoved, _TrackId);
    },
    [Location = *Location, Track = m_Timeline.GetTrack(_TrackId)](CTimelineDocument & _Document)
    {
      ExpectSuccess(_Document.m_Timeline.InsertTrack(Location, Track));
      _Document.Notify(ETimelineChangeKind::TrackAdded, Track.Id);
    });
}

void CTimelineDocument::MoveTrack(TTimelineId _TrackId, const TTimelineTrackLocation & _Location)
{
  const auto OldLocation = m_Timeline.FindTrackLocation(_TrackId);
  if (!OldLocation)
    return;

  if (_Location.ParentId != InvalidTimelineId && (!m_Timeline.HasTrack(_Location.ParentId) || m_Timeline.IsSameOrDescendant(_Location.ParentId, _TrackId)))
    return;

  const bool        IsSameParent = OldLocation->ParentId == _Location.ParentId;
  const std::size_t ChildCount   = GetChildCount(m_Timeline, _Location.ParentId) - (IsSameParent ? 1 : 0);

  if (_Location.Index > ChildCount || (IsSameParent && _Location.Index == OldLocation->Index))
    return;

  auto Move = [Track = m_Timeline.GetTrack(_TrackId)](const TTimelineTrackLocation & _Target)
  {
    return [Track, _Target](CTimelineDocument & _Document)
    {
      ExpectSuccess(_Document.m_Timeline.RemoveTrack(Track.Id));
      ExpectSuccess(_Document.m_Timeline.InsertTrack(_Target, Track));
      _Document.Notify(ETimelineChangeKind::TrackChanged, Track.Id);
    };
  };

  ExecuteCommand("Move Track", Move(_Location), Move(*OldLocation));
}

void CTimelineDocument::RenameTrack(TTimelineId _TrackId, const std::string & _Name)
{
  EditTracks("Rename Track", {_TrackId}, [&_Name](TTimelineTrack & _Track)
  {
    _Track.Name = _Name;
  });
}

void CTimelineDocument::SetTrackTarget(TTimelineId _TrackId, const TTimelineTarget & _Target)
{
  EditTracks("Set Track Target", {_TrackId}, [&_Target](TTimelineTrack & _Track)
  {
    _Track.Target = _Target;
  });
}

TTimelineId CTimelineDocument::AddKey(TTimelineId _TrackId, TTimelineKey _Key)
{
  if (!m_Timeline.HasTrack(_TrackId))
    return InvalidTimelineId;

  _Key.Id   = m_Timeline.MakeId();
  _Key.Time = std::max<TTimelineTicks>(_Key.Time, 0);

  EditTracks("Add Key", {_TrackId}, [&_Key](TTimelineTrack & _Track)
  {
    _Track.Keys.push_back(_Key);
  });

  return _Key.Id;
}

TTimelineId CTimelineDocument::AddClip(TTimelineId _TrackId, TTimelineClip _Clip)
{
  if (!m_Timeline.HasTrack(_TrackId))
    return InvalidTimelineId;

  _Clip.Id     = m_Timeline.MakeId();
  _Clip.Start  = std::max<TTimelineTicks>(_Clip.Start, 0);
  _Clip.Length = std::max<TTimelineTicks>(_Clip.Length, 0);

  EditTracks("Add Clip", {_TrackId}, [&_Clip](TTimelineTrack & _Track)
  {
    _Track.Clips.push_back(_Clip);
  });

  return _Clip.Id;
}

void CTimelineDocument::RemoveItems(const std::vector<TTimelineId> & _ItemIds)
{
  const std::unordered_set<TTimelineId> ItemIds(_ItemIds.begin(), _ItemIds.end());

  EditTracks("Delete Keys", FindItemTracks(_ItemIds), [&ItemIds](TTimelineTrack & _Track)
  {
    std::erase_if(_Track.Keys, [&ItemIds](const TTimelineKey & _Key)
    {
      return ItemIds.contains(_Key.Id);
    });

    std::erase_if(_Track.Clips, [&ItemIds](const TTimelineClip & _Clip)
    {
      return ItemIds.contains(_Clip.Id);
    });
  });
}

void CTimelineDocument::SetKeyValue(TTimelineId _KeyId, const TPropertyValue & _Value)
{
  EditItemTrack("Set Key Value", _KeyId, [_KeyId, &_Value](TTimelineTrack & _Track)
  {
    for (auto & Key : _Track.Keys)
    {
      if (Key.Id == _KeyId)
        Key.Value = _Value;
    }
  });
}

void CTimelineDocument::SetKeyInterpolation(TTimelineId _KeyId, ETimelineInterpolation _Interpolation)
{
  EditItemTrack("Set Key Interpolation", _KeyId, [_KeyId, _Interpolation](TTimelineTrack & _Track)
  {
    for (auto & Key : _Track.Keys)
    {
      if (Key.Id == _KeyId)
        Key.Interpolation = _Interpolation;
    }
  });
}

void CTimelineDocument::PreviewMoveItems(const std::vector<TTimelineId> & _ItemIds, TTimelineTicks _Delta)
{
  if (m_Preview && m_Preview->ItemIds != _ItemIds)
    CommitPreview();

  if (!m_Preview)
  {
    const auto TrackIds = FindItemTracks(_ItemIds);
    if (TrackIds.empty())
      return;

    m_Preview = TPreview{"Move Keys", _ItemIds, CaptureContents(TrackIds)};
  }

  const std::unordered_set<TTimelineId> ItemIds(_ItemIds.begin(), _ItemIds.end());
  const TTimelineTicks                  Delta = std::max(_Delta, -FindEarliestTime(m_Preview->Before, ItemIds));

  ApplyContents(m_Preview->Before);

  for (const auto & Content : m_Preview->Before)
  {
    m_Timeline.EditTrack(Content.Id, [&ItemIds, Delta](TTimelineTrack & _Track)
    {
      ShiftItems(_Track, ItemIds, Delta);
    });
  }
}

void CTimelineDocument::CommitPreview()
{
  if (!m_Preview)
    return;

  TPreview Preview = std::move(*m_Preview);
  m_Preview.reset();

  std::vector<TTimelineId> TrackIds;
  for (const auto & Content : Preview.Before)
  {
    if (m_Timeline.HasTrack(Content.Id))
      TrackIds.push_back(Content.Id);
  }

  auto After = CaptureContents(TrackIds);
  if (After == Preview.Before)
    return;

  ExecuteCommand(std::move(Preview.CommandName),
    [After = std::move(After)](CTimelineDocument & _Document)
    {
      _Document.ApplyContents(After);
    },
    [Before = std::move(Preview.Before)](CTimelineDocument & _Document)
    {
      _Document.ApplyContents(Before);
    });
}

void CTimelineDocument::CancelPreview()
{
  if (!m_Preview)
    return;

  const TPreview Preview = std::move(*m_Preview);
  m_Preview.reset();

  ApplyContents(Preview.Before);
}

//
// Signals
//

CTimelineDocument::TChangedSignal & CTimelineDocument::GetChangedSignal() noexcept
{
  return m_ChangedSignal;
}

//
// Service
//

void CTimelineDocument::ResetTimeline(CTimeline _Timeline)
{
  m_Preview.reset();
  m_Timeline = std::move(_Timeline);

  GetHistory().Clear();

  Notify(ETimelineChangeKind::TimelineChanged);
}

//
// Implementation
//

void CTimelineDocument::ExecuteCommand(std::string _CommandName, TEdit _Execute, TEdit _Undo)
{
  CommitPreview();

  auto Apply = [Document = weak_from_this()](const TEdit & _Edit)
  {
    if (const auto LockedDocument = Document.lock())
    {
      LockedDocument->m_Preview.reset();
      _Edit(*LockedDocument);
    }
  };

  GetHistory().Execute(std::make_unique<core::CFunctionCommand>(std::move(_CommandName),
    [Apply, Execute = std::move(_Execute)] { Apply(Execute); },
    [Apply, Undo = std::move(_Undo)] { Apply(Undo); }));
}

void CTimelineDocument::EditTracks(std::string _CommandName, const std::vector<TTimelineId> & _TrackIds, const TTrackEdit & _Edit)
{
  CommitPreview();

  auto Before = CaptureContents(_TrackIds);
  auto After  = Before;

  for (auto & Content : After)
    _Edit(Content);

  if (After == Before)
    return;

  ExecuteCommand(std::move(_CommandName),
    [After = std::move(After)](CTimelineDocument & _Document)
    {
      _Document.ApplyContents(After);
    },
    [Before = std::move(Before)](CTimelineDocument & _Document)
    {
      _Document.ApplyContents(Before);
    });
}

void CTimelineDocument::EditItemTrack(std::string _CommandName, TTimelineId _ItemId, const TTrackEdit & _Edit)
{
  if (const auto TrackId = m_Timeline.FindItemTrack(_ItemId))
    EditTracks(std::move(_CommandName), {*TrackId}, _Edit);
}

std::vector<TTimelineId> CTimelineDocument::FindItemTracks(const std::vector<TTimelineId> & _ItemIds) const
{
  std::vector<TTimelineId> TrackIds;

  for (const auto ItemId : _ItemIds)
  {
    const auto TrackId = m_Timeline.FindItemTrack(ItemId);
    if (TrackId && std::find(TrackIds.begin(), TrackIds.end(), *TrackId) == TrackIds.end())
      TrackIds.push_back(*TrackId);
  }

  return TrackIds;
}

std::vector<TTimelineTrack> CTimelineDocument::CaptureContents(const std::vector<TTimelineId> & _TrackIds) const
{
  std::vector<TTimelineTrack> Contents;

  for (const auto TrackId : _TrackIds)
  {
    if (m_Timeline.HasTrack(TrackId))
      Contents.push_back(m_Timeline.CaptureTrackContent(TrackId));
  }

  return Contents;
}

void CTimelineDocument::ApplyContents(const std::vector<TTimelineTrack> & _Contents)
{
  for (const auto & Content : _Contents)
  {
    ExpectSuccess(m_Timeline.ApplyTrackContent(Content));
    Notify(ETimelineChangeKind::TrackChanged, Content.Id);
  }
}

void CTimelineDocument::AssignIds(TTimelineTrack & _Track)
{
  _Track.Id = m_Timeline.MakeId();

  for (auto & Key : _Track.Keys)
    Key.Id = m_Timeline.MakeId();

  for (auto & Clip : _Track.Clips)
    Clip.Id = m_Timeline.MakeId();

  for (auto & Child : _Track.Children)
    AssignIds(Child);
}

void CTimelineDocument::Notify(ETimelineChangeKind _Kind, TTimelineId _TrackId) const
{
  m_ChangedSignal.Emit(TTimelineChange{_Kind, _TrackId});
}

} // namespace editor
