#pragma once

#include "TimelineDragMode.h"
#include "editor/timeline/model/TimelineTicks.h"

namespace editor
{

struct TTimelineDrag
{
  ETimelineDragMode Mode      = ETimelineDragMode::None;
  float             StartX    = 0.0f;
  float             StartY    = 0.0f;
  float             LaneLeft  = 0.0f;
  TTimelineTicks    Anchor    = 0;
  TTimelineTicks    Delta     = 0;
  bool              IsMoved   = false;
  bool              IsAdding  = false;
  bool              IsPending = false;
};

} // namespace editor
