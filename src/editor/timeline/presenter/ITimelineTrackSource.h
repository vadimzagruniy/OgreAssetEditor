#pragma once

#include "TimelineTrackTemplate.h"
#include "editor/inspector/PropertyValue.h"

#include <vector>

namespace editor
{

class CTimeline;

class ITimelineTrackSource
{
public: // Destructor

  virtual ~ITimelineTrackSource();

public: // Interface

  virtual std::vector<TTimelineTrackTemplate> GetTrackTemplates(const CTimeline & _Timeline) const = 0;

  virtual TPropertyValue CaptureValue(const TTimelineTrack & _Track) const = 0;
};

} // namespace editor
