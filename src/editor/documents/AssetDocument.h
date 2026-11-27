#pragma once

#include "core/UndoStack.h"

#include <memory>
#include <string>
#include <vector>

namespace editor
{

struct TAssetRecord;
struct TAssetProperty;

class CAssetDocument
{
public: // Constructor/Destructor

  CAssetDocument(TAssetRecord _Record, std::string _TypeName);

  virtual ~CAssetDocument();

  CAssetDocument(const CAssetDocument &) = delete;

  CAssetDocument & operator=(const CAssetDocument &) = delete;

public: // Interface

  const TAssetRecord & GetRecord() const noexcept;

  void SetRecord(TAssetRecord _Record);

  const std::string & GetTypeName() const noexcept;

  virtual std::vector<TAssetProperty> GetProperties() const = 0;

  core::CUndoStack & GetHistory() noexcept;

  const core::CUndoStack & GetHistory() const noexcept;

  bool IsDirty() const noexcept;

  bool Save(std::string & _Error);

  virtual void CommitPreview();

protected: // Events

  virtual bool OnSave(std::string & _Error);

private: // Attributes

  std::unique_ptr<TAssetRecord> m_Record;
  std::string                   m_TypeName;
  core::CUndoStack              m_History;
};

} // namespace editor
