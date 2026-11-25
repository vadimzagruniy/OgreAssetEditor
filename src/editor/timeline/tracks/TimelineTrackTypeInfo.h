#pragma once

#include "TimelineItemKind.h"

#include <string>

namespace editor
{

struct TTimelineTrackTypeInfo
{
  std::string       TypeId;
  std::string       Label;
  ETimelineItemKind ItemKind = ETimelineItemKind::Keys;
};

} // namespace editor
