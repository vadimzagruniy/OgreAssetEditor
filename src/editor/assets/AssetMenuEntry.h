#pragma once

#include "AssetMenuGroup.h"

#include <functional>
#include <string>

namespace editor
{

struct TAssetMenuEntry
{
  std::string           Label;
  std::string           Shortcut;
  EAssetMenuGroup       Group     = EAssetMenuGroup::Asset;
  bool                  IsEnabled = true;
  std::function<void()> Execute;
};

} // namespace editor
