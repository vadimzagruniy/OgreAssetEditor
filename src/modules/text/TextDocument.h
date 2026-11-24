#pragma once

#include "editor/documents/AssetDocument.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace editor
{

struct TTextChange;

class CTextDocument final:
  public CAssetDocument
{
public: // Constants

  static constexpr std::size_t MaxContentBytes = 16 * 1024 * 1024;

public: // Constructor

  CTextDocument(TAssetRecord _Record, std::string _Content);

public: // CAssetDocument

  std::vector<TAssetProperty> GetProperties() const override;

protected: // CAssetDocument

  bool OnSave(std::string & _Error) override;

public: // Interface

  const std::string & GetContent() const noexcept;

  std::uint64_t GetRevision() const noexcept;

  std::size_t GetLastChangeEnd() const noexcept;

  void Edit(TTextChange _Change, unsigned int _EditSession);

  void Replace(std::size_t _Offset, std::size_t _Count, const std::string & _Text);

private: // Attributes

  std::string   m_Content;
  std::uint64_t m_Revision;
  std::size_t   m_LastChangeEnd;
};

} // namespace editor
