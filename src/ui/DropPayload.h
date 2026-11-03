#pragma once

#include <string>

namespace editor::ui
{

struct TDropPayload
{
  std::string Data;
  bool        IsDelivered = false;
};

} // namespace editor::ui
