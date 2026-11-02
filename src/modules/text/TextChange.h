#pragma once

#include <cstddef>
#include <string>

namespace editor
{

struct TTextChange
{
  std::size_t Offset = 0;
  std::string RemovedText;
  std::string InsertedText;
};

} // namespace editor
