#pragma once

#include "editor/timeline/model/TimelineTicks.h"

namespace editor
{

enum class ETimelineEvaluateMode;
struct TTimelineTrack;

class ITimelineTrackEvaluator
{
public: // Destructor

  virtual ~ITimelineTrackEvaluator();

public: // Interface

  virtual void BeginPreview() = 0;

  virtual void Evaluate(const TTimelineTrack & _Track, TTimelineTicks _From, TTimelineTicks _To, ETimelineEvaluateMode _Mode) = 0;

  virtual void EndPreview() = 0;
};

} // namespace editor
