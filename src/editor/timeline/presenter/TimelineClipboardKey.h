#pragma once

#include "editor/timeline/model/TimelineId.h"
#include "editor/timeline/model/TimelineKey.h"

#include <string>

namespace editor
{

struct TTimelineClipboardKey
{
  TTimelineId  TrackId = InvalidTimelineId;
  std::string  TypeId;
  std::string  Property;
  TTimelineKey Key;
};

} // namespace editor
