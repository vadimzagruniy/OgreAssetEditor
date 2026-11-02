#pragma once

#include "editor/workspaces/AssetInstance.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace editor
{

class CTextInstance final:
  public CAssetInstance
{
public: // Constants

  static constexpr std::size_t MaxContentBytes = 16 * 1024 * 1024;

public: // Constructor

  CTextInstance(TAssetRecord _Record, std::string _Content);

public: // CAssetInstance

  std::vector<TAssetProperty> GetProperties() const override;

public: // Interface

  const std::string & GetContent() const noexcept;

  std::uint64_t GetRevision() const noexcept;

  std::size_t GetLastChangeEnd() const noexcept;

  void Replace(std::size_t _Offset, std::size_t _Count, const std::string & _Text);

  bool Save();

private: // Attributes

  std::string   m_Content;
  std::uint64_t m_Revision;
  std::size_t   m_LastChangeEnd;
};

} // namespace editor
