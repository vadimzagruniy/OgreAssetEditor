#include "AssetId.h"

#include <cstddef>
#include <random>

namespace editor
{

namespace
{

constexpr std::size_t TextLength  = 16;
constexpr const char  HexDigits[] = "0123456789abcdef";

bool ParseHexDigit(char _Character, std::uint64_t & _Digit)
{
  if (_Character >= '0' && _Character <= '9')
  {
    _Digit = static_cast<std::uint64_t>(_Character - '0');
    return true;
  }

  if (_Character >= 'a' && _Character <= 'f')
  {
    _Digit = static_cast<std::uint64_t>(_Character - 'a' + 10);
    return true;
  }

  if (_Character >= 'A' && _Character <= 'F')
  {
    _Digit = static_cast<std::uint64_t>(_Character - 'A' + 10);
    return true;
  }

  return false;
}

std::mt19937_64 MakeGenerator()
{
  std::random_device Device;
  std::seed_seq      Seed{Device(), Device(), Device(), Device()};

  return std::mt19937_64(Seed);
}

} // namespace

TAssetId TAssetId::Generate()
{
  static std::mt19937_64 Generator = MakeGenerator();

  TAssetId Id;
  while (!Id.IsValid())
    Id.Value = Generator();

  return Id;
}

bool TAssetId::FromString(std::string_view _Text, TAssetId & _Id)
{
  if (_Text.size() != TextLength)
    return false;

  std::uint64_t Value = 0;

  for (const char Character : _Text)
  {
    std::uint64_t Digit = 0;
    if (!ParseHexDigit(Character, Digit))
      return false;

    Value = (Value << 4) | Digit;
  }

  if (Value == 0)
    return false;

  _Id.Value = Value;

  return true;
}

bool TAssetId::IsValid() const noexcept
{
  return Value != 0;
}

std::string TAssetId::ToString() const
{
  std::string Text(TextLength, '0');

  for (std::size_t DigitIndex = 0; DigitIndex < TextLength; ++DigitIndex)
  {
    const auto Shift = static_cast<unsigned>((TextLength - 1 - DigitIndex) * 4);

    Text[DigitIndex] = HexDigits[(Value >> Shift) & 0xF];
  }

  return Text;
}

bool TAssetId::operator==(const TAssetId & _Other) const noexcept
{
  return Value == _Other.Value;
}

bool TAssetId::operator!=(const TAssetId & _Other) const noexcept
{
  return Value != _Other.Value;
}

bool TAssetId::operator<(const TAssetId & _Other) const noexcept
{
  return Value < _Other.Value;
}

} // namespace editor
