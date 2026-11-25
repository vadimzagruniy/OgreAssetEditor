#pragma once

#include "TimelineId.h"
#include "TimelineInterpolation.h"
#include "TimelineTicks.h"
#include "editor/inspector/PropertyValue.h"

namespace editor
{

struct TTimelineKey
{
  TTimelineId            Id            = InvalidTimelineId;
  TTimelineTicks         Time          = 0;
  TPropertyValue         Value;
  ETimelineInterpolation Interpolation = ETimelineInterpolation::Linear;

  bool operator==(const TTimelineKey &) const = default;
};

} // namespace editor
