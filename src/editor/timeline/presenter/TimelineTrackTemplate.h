#pragma once

#include "editor/timeline/model/TimelineTrack.h"

#include <string>

namespace editor
{

struct TTimelineTrackTemplate
{
  std::string    Label;
  TTimelineTrack Track;
};

} // namespace editor
