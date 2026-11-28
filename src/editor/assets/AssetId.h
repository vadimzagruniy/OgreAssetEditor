#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace editor
{

struct TAssetId
{
  std::uint64_t Value = 0;

  static TAssetId Generate();

  static bool FromString(std::string_view _Text, TAssetId & _Id);

  bool IsValid() const noexcept;

  std::string ToString() const;

  bool operator==(const TAssetId & _Other) const noexcept;

  bool operator!=(const TAssetId & _Other) const noexcept;

  bool operator<(const TAssetId & _Other) const noexcept;
};

} // namespace editor
