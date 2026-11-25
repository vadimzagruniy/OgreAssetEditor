#pragma once

#include "editor/inspector/PropertyValue.h"
#include "editor/timeline/model/TimelineKey.h"

#include <vector>

namespace editor
{

TPropertyValue EvaluateKeys(const std::vector<TTimelineKey> & _Keys, TTimelineTicks _Time);

} // namespace editor
