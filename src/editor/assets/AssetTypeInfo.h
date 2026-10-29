#pragma once

#include <string>
#include <vector>

namespace editor
{

struct TAssetTypeInfo
{
  std::string              Identifier;
  std::string              Name;
  std::vector<std::string> Extensions;
  bool                     Editable = true;
};

} // namespace editor
