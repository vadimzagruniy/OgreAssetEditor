#pragma once

#include "editor/inspector/PropertyMeta.h"
#include "editor/inspector/PropertyType.h"

#include <string>
#include <vector>

namespace editor
{

struct TPropertyDesc
{
  std::string                Identifier;
  std::string                Label;
  EPropertyType              Type = EPropertyType::Group;
  TPropertyMeta              Meta;
  std::vector<TPropertyDesc> Children;
};

} // namespace editor
