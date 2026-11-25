#include "TimelineViewRange.h"
#include "TimelineTime.h"
#include "TimelineTimeFormat.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace editor
{

namespace
{

constexpr double DefaultPixelsPerSecond = 200.0;
constexpr double MinPixelsPerSecond     = 4.0;
constexpr double MaxPixelsPerSecond     = 20000.0;
constexpr float  FitMargin              = 0.05f;

constexpr std::array<TTimelineTicks, 9> StepMultipliers{1, 2, 5, 10, 20, 50, 100, 200, 500};

} // namespace

//
// Constructor
//

CTimelineViewRange::CTimelineViewRange():
  m_Start(0.0),
  m_PixelsPerSecond(DefaultPixelsPerSecond)
{
  // Empty
}

//
// Interface
//

float CTimelineViewRange::ToOffset(TTimelineTicks _Time) const noexcept
{
  return static_cast<float>((static_cast<double>(_Time) - m_Start) * m_PixelsPerSecond / static_cast<double>(TimelineTicksPerSecond));
}

TTimelineTicks CTimelineViewRange::ToTime(float _Offset) const noexcept
{
  return static_cast<TTimelineTicks>(std::llround(m_Start + static_cast<double>(_Offset) * static_cast<double>(TimelineTicksPerSecond) / m_PixelsPerSecond));
}

void CTimelineViewRange::Scroll(float _Pixels) noexcept
{
  m_Start = std::max(0.0, m_Start + static_cast<double>(_Pixels) * static_cast<double>(TimelineTicksPerSecond) / m_PixelsPerSecond);
}

void CTimelineViewRange::Zoom(float _Factor, float _AnchorOffset) noexcept
{
  const auto AnchorTime = static_cast<double>(ToTime(_AnchorOffset));

  m_PixelsPerSecond = std::clamp(m_PixelsPerSecond * static_cast<double>(_Factor), MinPixelsPerSecond, MaxPixelsPerSecond);
  m_Start           = std::max(0.0, AnchorTime - static_cast<double>(_AnchorOffset) * static_cast<double>(TimelineTicksPerSecond) / m_PixelsPerSecond);
}

void CTimelineViewRange::Fit(TTimelineTicks _Duration, float _Width) noexcept
{
  if (_Duration <= 0 || _Width <= 0.0f)
    return;

  const double Seconds = static_cast<double>(_Duration) / static_cast<double>(TimelineTicksPerSecond);

  m_Start           = 0.0;
  m_PixelsPerSecond = std::clamp(static_cast<double>(_Width * (1.0f - FitMargin)) / Seconds, MinPixelsPerSecond, MaxPixelsPerSecond);
}

TTimelineTicks CTimelineViewRange::FindGridStep(ETimelineTimeFormat _Format, int _FrameRate, float _MinSpacing) const
{
  const TTimelineTicks BaseStep = _Format == ETimelineTimeFormat::Frames ? GetFrameTicks(_FrameRate) : TimelineTicksPerSecond / 100;
  const double         MinTicks = static_cast<double>(_MinSpacing) * static_cast<double>(TimelineTicksPerSecond) / m_PixelsPerSecond;

  for (TTimelineTicks Scale = 1; Scale <= 1000; Scale *= 1000)
  {
    for (const auto Multiplier : StepMultipliers)
    {
      const TTimelineTicks Step = BaseStep * Multiplier * Scale;
      if (static_cast<double>(Step) >= MinTicks)
        return Step;
    }
  }

  return BaseStep * StepMultipliers.back() * 1000;
}

} // namespace editor
