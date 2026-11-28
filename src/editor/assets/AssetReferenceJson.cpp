#include "AssetReferenceJson.h"
#include "AssetReference.h"

#include <nlohmann/json.hpp>

namespace editor
{

namespace
{

const std::string IdKey   = "id";
const std::string PathKey = "path";

bool ReadReferenceObject(const TJson & _Json, const std::string & _Key, TAssetReference & _AssetRef, std::string & _Error)
{
  TAssetReference AssetRef;

  if (const auto IdField = _Json.find(IdKey); IdField != _Json.end())
  {
    if (!IdField->is_string() || !TAssetId::FromString(IdField->get<std::string>(), AssetRef.Id))
    {
      _Error = "'" + _Key + "." + IdKey + "' must be 16 hexadecimal digits and not zero.";
      return false;
    }
  }

  if (const auto PathField = _Json.find(PathKey); PathField != _Json.end())
  {
    if (!PathField->is_string())
    {
      _Error = "'" + _Key + "." + PathKey + "' must be a string.";
      return false;
    }

    AssetRef.PathHint = PathField->get<std::string>();
  }

  _AssetRef = AssetRef;

  return true;
}

} // namespace

bool ReadAssetReference(const TJson & _Object, const std::string & _Key, TAssetReference & _AssetRef, std::string & _Error)
{
  const auto Field = _Object.find(_Key);
  if (Field == _Object.end())
    return true;

  if (Field->is_string())
  {
    _AssetRef = TAssetReference{{}, Field->get<std::string>()};
    return true;
  }

  if (!Field->is_object())
  {
    _Error = "'" + _Key + "' must be an asset reference object.";
    return false;
  }

  return ReadReferenceObject(*Field, _Key, _AssetRef, _Error);
}

TJson WriteAssetReference(const TAssetReference & _AssetRef)
{
  TJson Json = TJson::object();

  if (_AssetRef.Id.IsValid())
    Json[IdKey] = _AssetRef.Id.ToString();

  if (!_AssetRef.PathHint.empty())
    Json[PathKey] = _AssetRef.PathHint;

  return Json;
}

} // namespace editor
