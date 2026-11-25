#include "Timeline.h"

#include <algorithm>
#include <cassert>
#include <utility>

namespace editor
{

namespace
{

constexpr TTimelineTicks DefaultDuration  = 5 * TimelineTicksPerSecond;
constexpr int            DefaultFrameRate = 30;

std::optional<TTimelineTrackLocation> FindLocation(const std::vector<TTimelineTrack> & _Tracks, TTimelineId _ParentId, TTimelineId _TrackId)
{
  for (std::size_t TrackIndex = 0; TrackIndex < _Tracks.size(); ++TrackIndex)
  {
    const auto & Track = _Tracks[TrackIndex];

    if (Track.Id == _TrackId)
      return TTimelineTrackLocation{_ParentId, TrackIndex};

    if (auto Location = FindLocation(Track.Children, Track.Id, _TrackId))
      return Location;
  }

  return std::nullopt;
}

bool ContainsItem(const TTimelineTrack & _Track, TTimelineId _ItemId)
{
  const bool HasKey = std::any_of(_Track.Keys.begin(), _Track.Keys.end(), [_ItemId](const TTimelineKey & _Key)
  {
    return _Key.Id == _ItemId;
  });

  return HasKey || std::any_of(_Track.Clips.begin(), _Track.Clips.end(), [_ItemId](const TTimelineClip & _Clip)
  {
    return _Clip.Id == _ItemId;
  });
}

std::optional<TTimelineId> FindItemOwner(const std::vector<TTimelineTrack> & _Tracks, TTimelineId _ItemId)
{
  for (const auto & Track : _Tracks)
  {
    if (ContainsItem(Track, _ItemId))
      return Track.Id;

    if (auto Owner = FindItemOwner(Track.Children, _ItemId))
      return Owner;
  }

  return std::nullopt;
}

TTimelineId FindMaxId(const TTimelineTrack & _Track)
{
  TTimelineId MaxId = _Track.Id;

  for (const auto & Key : _Track.Keys)
    MaxId = std::max(MaxId, Key.Id);

  for (const auto & Clip : _Track.Clips)
    MaxId = std::max(MaxId, Clip.Id);

  for (const auto & Child : _Track.Children)
    MaxId = std::max(MaxId, FindMaxId(Child));

  return MaxId;
}

void SortItems(TTimelineTrack & _Track)
{
  std::stable_sort(_Track.Keys.begin(), _Track.Keys.end(), [](const TTimelineKey & _Left, const TTimelineKey & _Right)
  {
    return _Left.Time < _Right.Time;
  });

  std::stable_sort(_Track.Clips.begin(), _Track.Clips.end(), [](const TTimelineClip & _Left, const TTimelineClip & _Right)
  {
    return _Left.Start < _Right.Start;
  });
}

} // namespace

//
// Constructor
//

CTimeline::CTimeline():
  m_Duration(DefaultDuration),
  m_FrameRate(DefaultFrameRate),
  m_NextId(InvalidTimelineId + 1)
{
  // Empty
}

//
// Interface
//

TTimelineTicks CTimeline::GetDuration() const noexcept
{
  return m_Duration;
}

void CTimeline::SetDuration(TTimelineTicks _Duration) noexcept
{
  m_Duration = std::max<TTimelineTicks>(_Duration, 0);
}

int CTimeline::GetFrameRate() const noexcept
{
  return m_FrameRate;
}

void CTimeline::SetFrameRate(int _FrameRate) noexcept
{
  m_FrameRate = std::max(_FrameRate, 1);
}

const std::vector<TTimelineTrack> & CTimeline::GetTracks() const noexcept
{
  return m_Tracks;
}

TTimelineId CTimeline::MakeId() noexcept
{
  return m_NextId++;
}

bool CTimeline::HasTrack(TTimelineId _TrackId) const
{
  return FindTrackLocation(_TrackId).has_value();
}

const TTimelineTrack & CTimeline::GetTrack(TTimelineId _TrackId) const
{
  const auto Location = FindTrackLocation(_TrackId);
  assert(Location && "Timeline track is missing");

  const auto & Siblings = Location->ParentId == InvalidTimelineId ? m_Tracks : GetTrack(Location->ParentId).Children;

  return Siblings[Location->Index];
}

std::optional<TTimelineTrackLocation> CTimeline::FindTrackLocation(TTimelineId _TrackId) const
{
  if (_TrackId == InvalidTimelineId)
    return std::nullopt;

  return FindLocation(m_Tracks, InvalidTimelineId, _TrackId);
}

std::optional<TTimelineId> CTimeline::FindItemTrack(TTimelineId _ItemId) const
{
  if (_ItemId == InvalidTimelineId)
    return std::nullopt;

  return FindItemOwner(m_Tracks, _ItemId);
}

bool CTimeline::IsSameOrDescendant(TTimelineId _TrackId, TTimelineId _AncestorId) const
{
  TTimelineId CurrentId = _TrackId;

  while (CurrentId != InvalidTimelineId)
  {
    if (CurrentId == _AncestorId)
      return true;

    const auto Location = FindTrackLocation(CurrentId);
    if (!Location)
      return false;

    CurrentId = Location->ParentId;
  }

  return false;
}

bool CTimeline::InsertTrack(const TTimelineTrackLocation & _Location, TTimelineTrack _Track)
{
  if (_Track.Id == InvalidTimelineId || HasTrack(_Track.Id))
    return false;

  if (_Location.ParentId != InvalidTimelineId && !HasTrack(_Location.ParentId))
    return false;

  auto & Siblings = GetMutableSiblings(_Location.ParentId);
  if (_Location.Index > Siblings.size())
    return false;

  ReserveIds(_Track);
  SortItems(_Track);

  Siblings.insert(Siblings.begin() + static_cast<std::ptrdiff_t>(_Location.Index), std::move(_Track));

  return true;
}

bool CTimeline::RemoveTrack(TTimelineId _TrackId)
{
  const auto Location = FindTrackLocation(_TrackId);
  if (!Location)
    return false;

  auto & Siblings = GetMutableSiblings(Location->ParentId);
  Siblings.erase(Siblings.begin() + static_cast<std::ptrdiff_t>(Location->Index));

  return true;
}

bool CTimeline::EditTrack(TTimelineId _TrackId, const std::function<void(TTimelineTrack &)> & _Edit)
{
  if (!HasTrack(_TrackId))
    return false;

  auto & Track = GetMutableTrack(_TrackId);

  _Edit(Track);
  SortItems(Track);
  ReserveIds(Track);

  return true;
}

TTimelineTrack CTimeline::CaptureTrackContent(TTimelineId _TrackId) const
{
  TTimelineTrack Content = GetTrack(_TrackId);
  Content.Children.clear();

  return Content;
}

bool CTimeline::ApplyTrackContent(const TTimelineTrack & _Content)
{
  if (!HasTrack(_Content.Id))
    return false;

  auto & Track    = GetMutableTrack(_Content.Id);
  auto   Children = std::move(Track.Children);

  Track          = _Content;
  Track.Children = std::move(Children);

  SortItems(Track);
  ReserveIds(Track);

  return true;
}

//
// Implementation
//

TTimelineTrack & CTimeline::GetMutableTrack(TTimelineId _TrackId)
{
  const auto Location = FindTrackLocation(_TrackId);
  assert(Location && "Timeline track is missing");

  return GetMutableSiblings(Location->ParentId)[Location->Index];
}

std::vector<TTimelineTrack> & CTimeline::GetMutableSiblings(TTimelineId _ParentId)
{
  return _ParentId == InvalidTimelineId ? m_Tracks : GetMutableTrack(_ParentId).Children;
}

void CTimeline::ReserveIds(const TTimelineTrack & _Track) noexcept
{
  m_NextId = std::max(m_NextId, FindMaxId(_Track) + 1);
}

} // namespace editor
