#pragma once

#include "editor/timeline/model/TimelineTicks.h"

namespace editor
{

class CTimeline;
enum class ETimelineEvaluateMode;

class ITimelineEvaluator
{
public: // Destructor

  virtual ~ITimelineEvaluator();

public: // Interface

  virtual void BeginPreview() = 0;

  virtual void Evaluate(const CTimeline & _Timeline, TTimelineTicks _From, TTimelineTicks _To, ETimelineEvaluateMode _Mode) = 0;

  virtual void EndPreview() = 0;
};

} // namespace editor
