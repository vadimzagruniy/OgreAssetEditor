#pragma once

#include <cstdint>

namespace editor
{

using TTimelineTicks = std::int64_t;

inline constexpr TTimelineTicks TimelineTicksPerSecond = 60000;

} // namespace editor
