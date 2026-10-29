#pragma once

#include "editor/workspaces/AssetInstance.h"

#include <cstddef>
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

public: // Interface

  std::string & GetContent() noexcept;

  bool IsDirty() const noexcept;

  void MarkDirty() noexcept;

  bool Save();

public: // CAssetInstance

  std::vector<TAssetProperty> GetProperties() const override;

private: // Attributes

  std::string m_Content;
  bool        m_Dirty = false;
};

} // namespace editor
