#include "AssetNameRules.h"
#include "AssetPath.h"

#include <algorithm>
#include <array>

namespace editor
{

namespace
{

constexpr std::size_t MaxNameLength = 128;

constexpr std::array<std::string_view, 22> ReservedNames =
{
  "CON", "PRN", "AUX", "NUL",
  "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
  "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
};

bool IsAllowedCharacter(char _Character)
{
  if ((_Character >= 'a' && _Character <= 'z') || (_Character >= 'A' && _Character <= 'Z') || (_Character >= '0' && _Character <= '9'))
    return true;

  return _Character == ' ' || _Character == '_' || _Character == '-' || _Character == '.';
}

char ToUpper(char _Character)
{
  return _Character >= 'a' && _Character <= 'z' ? static_cast<char>(_Character - 'a' + 'A') : _Character;
}

bool IsReservedName(std::string_view _Name)
{
  const std::string_view Stem = _Name.substr(0, _Name.find('.'));

  return std::any_of(ReservedNames.begin(), ReservedNames.end(), [Stem](std::string_view _Reserved)
  {
    return std::equal(Stem.begin(), Stem.end(), _Reserved.begin(), _Reserved.end(), [](char _Left, char _Right)
    {
      return ToUpper(_Left) == _Right;
    });
  });
}

} // namespace

bool IsValidAssetName(std::string_view _Name, std::string & _Error)
{
  if (_Name.empty())
  {
    _Error = "Enter a name.";
    return false;
  }

  if (_Name.size() > MaxNameLength)
  {
    _Error = "The name is longer than " + std::to_string(MaxNameLength) + " characters.";
    return false;
  }

  if (!std::all_of(_Name.begin(), _Name.end(), IsAllowedCharacter))
  {
    _Error = "Use only Latin letters, digits, spaces and _ - .";
    return false;
  }

  if (_Name.front() == ' ' || _Name.front() == '.' || _Name.back() == ' ' || _Name.back() == '.')
  {
    _Error = "The name cannot start or end with a space or a dot.";
    return false;
  }

  if (IsReservedName(_Name))
  {
    _Error = "The name is reserved by the system.";
    return false;
  }

  if (IsMetaPath(std::filesystem::path(std::string(_Name))))
  {
    _Error = "The .meta extension is reserved for asset metadata.";
    return false;
  }

  return true;
}

} // namespace editor
