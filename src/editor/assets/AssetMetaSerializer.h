#pragma once

#include <filesystem>
#include <string>

namespace editor
{

struct TAssetMeta;

bool ReadAssetMeta(const std::string & _Text, TAssetMeta & _Meta, std::string & _Error);

std::string WriteAssetMeta(const TAssetMeta & _Meta);

bool LoadAssetMeta(const std::filesystem::path & _MetaPath, TAssetMeta & _Meta, std::string & _Error);

bool SaveAssetMeta(const std::filesystem::path & _MetaPath, const TAssetMeta & _Meta, std::string & _Error);

} // namespace editor
