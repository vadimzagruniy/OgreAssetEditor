#include "TimelineTime.h"
#include "TimelineTimeFormat.h"

#include <algorithm>
#include <cmath>
#include <format>

namespace editor
{

TTimelineTicks GetFrameTicks(int _FrameRate)
{
  return std::max<TTimelineTicks>(TimelineTicksPerSecond / std::max(_FrameRate, 1), 1);
}

TTimelineTicks SnapToFrame(TTimelineTicks _Time, int _FrameRate)
{
  const double FrameTicks = static_cast<double>(TimelineTicksPerSecond) / static_cast<double>(std::max(_FrameRate, 1));
  const double Frame      = std::round(static_cast<double>(_Time) / FrameTicks);

  return static_cast<TTimelineTicks>(std::llround(Frame * FrameTicks));
}

std::string FormatTimelineTime(TTimelineTicks _Time, ETimelineTimeFormat _Format, int _FrameRate)
{
  const double Seconds = static_cast<double>(_Time) / static_cast<double>(TimelineTicksPerSecond);

  if (_Format == ETimelineTimeFormat::Frames)
    return std::to_string(std::llround(Seconds * std::max(_FrameRate, 1)));

  return std::format("{:g}s", std::round(Seconds * 1000.0) / 1000.0);
}

} // namespace editor
