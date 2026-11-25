#pragma once

#include "editor/timeline/model/TimelineId.h"

namespace editor
{

struct TTimelineKeyHit
{
  TTimelineId Id        = InvalidTimelineId;
  float       PositionX = 0.0f;
  float       PositionY = 0.0f;
};

} // namespace editor
