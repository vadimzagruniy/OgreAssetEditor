#pragma once

#include "TimelineId.h"
#include "TimelineKey.h"

namespace editor
{

struct TTimelineTrackKey
{
  TTimelineId  TrackId = InvalidTimelineId;
  TTimelineKey Key;
};

} // namespace editor
