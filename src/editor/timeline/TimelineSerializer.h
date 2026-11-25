#pragma once

#include "editor/json/Json.h"

#include <string>

namespace editor
{

class CTimeline;

bool ReadTimeline(const TJson & _Json, CTimeline & _Timeline, std::string & _Error);

TJson WriteTimeline(const CTimeline & _Timeline);

} // namespace editor
