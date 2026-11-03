#include "AxisColors.h"

#include <array>

namespace editor
{

ImU32 GetAxisColor(std::size_t _AxisIndex)
{
  static const std::array<ImU32, 3> Colors{IM_COL32(224, 79, 95, 255), IM_COL32(124, 179, 66, 255), IM_COL32(62, 142, 222, 255)};

  return Colors[_AxisIndex];
}

} // namespace editor
