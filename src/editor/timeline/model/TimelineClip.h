#pragma once

#include "TimelineId.h"
#include "TimelineTicks.h"

#include <string>

namespace editor
{

struct TTimelineClip
{
  TTimelineId    Id     = InvalidTimelineId;
  TTimelineTicks Start  = 0;
  TTimelineTicks Length = 0;
  TTimelineTicks Offset = 0;
  std::string    Source;

  bool operator==(const TTimelineClip &) const = default;
};

} // namespace editor
