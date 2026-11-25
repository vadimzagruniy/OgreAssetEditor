#pragma once

#include <string>

namespace editor
{

struct TTimelineTarget
{
  std::string Slot;
  std::string Property;

  bool operator==(const TTimelineTarget &) const = default;
};

} // namespace editor
