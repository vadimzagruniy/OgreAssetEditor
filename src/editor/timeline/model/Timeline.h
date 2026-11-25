#pragma once

#include "TimelineId.h"
#include "TimelineTicks.h"
#include "TimelineTrack.h"
#include "TimelineTrackLocation.h"

#include <functional>
#include <optional>
#include <vector>

namespace editor
{

class CTimeline
{
public: // Constructor

  CTimeline();

public: // Interface

  TTimelineTicks GetDuration() const noexcept;

  void SetDuration(TTimelineTicks _Duration) noexcept;

  int GetFrameRate() const noexcept;

  void SetFrameRate(int _FrameRate) noexcept;

  const std::vector<TTimelineTrack> & GetTracks() const noexcept;

  TTimelineId MakeId() noexcept;

  bool HasTrack(TTimelineId _TrackId) const;

  const TTimelineTrack & GetTrack(TTimelineId _TrackId) const;

  std::optional<TTimelineTrackLocation> FindTrackLocation(TTimelineId _TrackId) const;

  std::optional<TTimelineId> FindItemTrack(TTimelineId _ItemId) const;

  bool IsSameOrDescendant(TTimelineId _TrackId, TTimelineId _AncestorId) const;

  bool InsertTrack(const TTimelineTrackLocation & _Location, TTimelineTrack _Track);

  bool RemoveTrack(TTimelineId _TrackId);

  bool EditTrack(TTimelineId _TrackId, const std::function<void(TTimelineTrack &)> & _Edit);

  TTimelineTrack CaptureTrackContent(TTimelineId _TrackId) const;

  bool ApplyTrackContent(const TTimelineTrack & _Content);

private: // Implementation

  TTimelineTrack & GetMutableTrack(TTimelineId _TrackId);

  std::vector<TTimelineTrack> & GetMutableSiblings(TTimelineId _ParentId);

  void ReserveIds(const TTimelineTrack & _Track) noexcept;

private: // Attributes

  TTimelineTicks              m_Duration;
  int                         m_FrameRate;
  std::vector<TTimelineTrack> m_Tracks;
  TTimelineId                 m_NextId;
};

} // namespace editor
