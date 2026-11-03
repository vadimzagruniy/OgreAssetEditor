#pragma once

#include "editor/inspector/PropertyDesc.h"

#include <string>
#include <vector>

namespace editor
{

struct TPropertySchema
{
  std::string                Identifier;
  std::vector<TPropertyDesc> Properties;
};

} // namespace editor
