#pragma once

#include <string>
#include <vector>

namespace editor
{

struct TPropertyMeta
{
  float                    Minimum        = 0.0f;
  float                    Maximum        = 0.0f;
  float                    Speed          = 0.1f;
  std::string              Format         = "%.3f";
  std::string              Hint;
  std::vector<std::string> Options;
  std::string              AssetType;
  bool                     IsAlphaEnabled = false;
  bool                     IsReadOnly     = false;
};

} // namespace editor
