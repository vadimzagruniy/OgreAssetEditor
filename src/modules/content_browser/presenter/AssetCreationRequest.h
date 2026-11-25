#pragma once

#include <filesystem>
#include <string>

namespace editor
{

struct TAssetCreationRequest
{
  std::string           FactoryIdentifier;
  std::string           Label;
  std::filesystem::path Folder;
  std::string           DefaultName;
};

} // namespace editor
