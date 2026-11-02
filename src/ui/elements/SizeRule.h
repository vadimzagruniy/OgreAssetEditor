#pragma once

#include "ui/elements/SizeMode.h"

namespace editor::ui
{

struct TSizeRule
{
  ESizeMode Mode  = ESizeMode::Auto;
  float     Value = 0.0f;
};

} // namespace editor::ui
