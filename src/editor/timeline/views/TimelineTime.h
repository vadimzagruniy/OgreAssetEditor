#pragma once

#include "editor/timeline/model/TimelineTicks.h"

#include <string>

namespace editor
{

enum class ETimelineTimeFormat;

TTimelineTicks GetFrameTicks(int _FrameRate);

TTimelineTicks SnapToFrame(TTimelineTicks _Time, int _FrameRate);

std::string FormatTimelineTime(TTimelineTicks _Time, ETimelineTimeFormat _Format, int _FrameRate);

} // namespace editor
