#pragma once

#include "editor/timeline/playback/ITimelineEvaluator.h"

#include <memory>
#include <vector>

namespace editor
{

class CTimelineTrackTypes;
struct TTimelineTrack;

class CTimelineTrackEvaluation final:
  public ITimelineEvaluator
{
public: // Constructor

  explicit CTimelineTrackEvaluation(std::shared_ptr<const CTimelineTrackTypes> _TrackTypes);

public: // ITimelineEvaluator

  void BeginPreview() override;

  void Evaluate(const CTimeline & _Timeline, TTimelineTicks _From, TTimelineTicks _To, ETimelineEvaluateMode _Mode) override;

  void EndPreview() override;

private: // Implementation

  void EvaluateTracks(const std::vector<TTimelineTrack> & _Tracks, TTimelineTicks _From, TTimelineTicks _To, ETimelineEvaluateMode _Mode) const;

private: // Attributes

  std::shared_ptr<const CTimelineTrackTypes> m_TrackTypes;
};

} // namespace editor
