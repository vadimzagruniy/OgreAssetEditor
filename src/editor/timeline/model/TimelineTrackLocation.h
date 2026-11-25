#pragma once

#include "TimelineId.h"

#include <cstddef>

namespace editor
{

struct TTimelineTrackLocation
{
  TTimelineId ParentId = InvalidTimelineId;
  std::size_t Index    = 0;
};

} // namespace editor
