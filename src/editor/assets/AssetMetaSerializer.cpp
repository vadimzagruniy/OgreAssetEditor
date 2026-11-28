#include "AssetMetaSerializer.h"
#include "AssetMeta.h"
#include "core/ReadFileText.h"
#include "core/WriteFileAtomic.h"
#include "editor/json/Json.h"

#include <nlohmann/json.hpp>

#include <cstdint>

namespace editor
{

namespace
{

constexpr std::int64_t MetaFormatVersion = 1;

const std::string VersionKey = "version";
const std::string IdKey      = "id";

} // namespace

bool ReadAssetMeta(const std::string & _Text, TAssetMeta & _Meta, std::string & _Error)
{
  const auto Root = TJson::parse(_Text, nullptr, false);
  if (Root.is_discarded() || !Root.is_object())
  {
    _Error = "Metadata is not a valid JSON object.";
    return false;
  }

  const auto VersionField = Root.find(VersionKey);
  if (VersionField == Root.end() || !VersionField->is_number_integer())
  {
    _Error = "Metadata has no integer 'version'.";
    return false;
  }

  const auto Version = VersionField->get<std::int64_t>();
  if (Version < 1 || Version > MetaFormatVersion)
  {
    _Error = "Metadata version " + std::to_string(Version) + " is not supported.";
    return false;
  }

  const auto IdField = Root.find(IdKey);
  if (IdField == Root.end() || !IdField->is_string())
  {
    _Error = "Metadata has no string 'id'.";
    return false;
  }

  TAssetMeta Meta;
  if (!TAssetId::FromString(IdField->get<std::string>(), Meta.Id))
  {
    _Error = "Metadata 'id' must be 16 hexadecimal digits and not zero.";
    return false;
  }

  _Meta = Meta;

  return true;
}

std::string WriteAssetMeta(const TAssetMeta & _Meta)
{
  TJson Root = TJson::object();

  Root[VersionKey] = MetaFormatVersion;
  Root[IdKey]      = _Meta.Id.ToString();

  return Root.dump(2, ' ', false, TJson::error_handler_t::replace) + "\n";
}

bool LoadAssetMeta(const std::filesystem::path & _MetaPath, TAssetMeta & _Meta, std::string & _Error)
{
  std::string Text;

  return core::ReadFileText(_MetaPath, Text, _Error) && ReadAssetMeta(Text, _Meta, _Error);
}

bool SaveAssetMeta(const std::filesystem::path & _MetaPath, const TAssetMeta & _Meta, std::string & _Error)
{
  return core::WriteFileAtomic(_MetaPath, WriteAssetMeta(_Meta), _Error);
}

} // namespace editor
