#include "TimelineTrackEvaluation.h"
#include "ITimelineTrackEvaluator.h"
#include "TimelineTrackTypes.h"
#include "editor/timeline/model/Timeline.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CTimelineTrackEvaluation::CTimelineTrackEvaluation(std::shared_ptr<const CTimelineTrackTypes> _TrackTypes):
  m_TrackTypes(std::move(_TrackTypes))
{
  // Empty
}

//
// ITimelineEvaluator
//

void CTimelineTrackEvaluation::BeginPreview()
{
  for (const auto & Evaluator : m_TrackTypes->GetEvaluators())
    Evaluator->BeginPreview();
}

void CTimelineTrackEvaluation::Evaluate(const CTimeline & _Timeline, TTimelineTicks _From, TTimelineTicks _To, ETimelineEvaluateMode _Mode)
{
  EvaluateTracks(_Timeline.GetTracks(), _From, _To, _Mode);
}

void CTimelineTrackEvaluation::EndPreview()
{
  for (const auto & Evaluator : m_TrackTypes->GetEvaluators())
    Evaluator->EndPreview();
}

//
// Implementation
//

void CTimelineTrackEvaluation::EvaluateTracks(const std::vector<TTimelineTrack> & _Tracks, TTimelineTicks _From, TTimelineTicks _To, ETimelineEvaluateMode _Mode) const
{
  for (const auto & Track : _Tracks)
  {
    if (const auto Evaluator = m_TrackTypes->FindEvaluator(Track.TypeId))
      Evaluator->Evaluate(Track, _From, _To, _Mode);

    EvaluateTracks(Track.Children, _From, _To, _Mode);
  }
}

} // namespace editor
