#pragma once

#include "TimelineClip.h"
#include "TimelineId.h"
#include "TimelineKey.h"
#include "TimelineTarget.h"

#include <string>
#include <vector>

namespace editor
{

struct TTimelineTrack
{
  TTimelineId                 Id = InvalidTimelineId;
  std::string                 TypeId;
  std::string                 Name;
  TTimelineTarget             Target;
  std::vector<TTimelineKey>   Keys;
  std::vector<TTimelineClip>  Clips;
  std::vector<TTimelineTrack> Children;

  bool operator==(const TTimelineTrack &) const = default;
};

} // namespace editor
