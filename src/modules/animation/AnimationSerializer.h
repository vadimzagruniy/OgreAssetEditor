#pragma once

#include <string>

namespace editor
{

class CTimeline;

bool ReadAnimation(const std::string & _Text, CTimeline & _Timeline, std::string & _Error);

std::string WriteAnimation(const CTimeline & _Timeline);

} // namespace editor
