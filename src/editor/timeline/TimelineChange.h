#pragma once

#include "TimelineChangeKind.h"
#include "editor/timeline/model/TimelineId.h"

namespace editor
{

struct TTimelineChange
{
  ETimelineChangeKind Kind    = ETimelineChangeKind::TrackChanged;
  TTimelineId         TrackId = InvalidTimelineId;
};

} // namespace editor
