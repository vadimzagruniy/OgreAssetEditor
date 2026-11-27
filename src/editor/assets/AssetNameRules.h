#pragma once

#include <string>
#include <string_view>

namespace editor
{

bool IsValidAssetName(std::string_view _Name, std::string & _Error);

} // namespace editor
