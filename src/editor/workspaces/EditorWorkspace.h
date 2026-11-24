#pragma once

#include "core/TypeKey.h"

#include <memory>
#include <string>
#include <vector>

namespace editor
{

class CAssetDocument;
class IPropertySource;

class CEditorWorkspace
{
public: // Constructor/Destructor

  explicit CEditorWorkspace(std::shared_ptr<CAssetDocument> _Document);

  virtual ~CEditorWorkspace();

  CEditorWorkspace(const CEditorWorkspace &) = delete;

  CEditorWorkspace & operator=(const CEditorWorkspace &) = delete;

public: // Interface

  CAssetDocument & GetDocument() const noexcept;

  std::string GetTitle() const;

  bool IsDirty() const;

  bool Save(std::string & _Error);

  virtual std::vector<core::TTypeKey> GetToolBoxes() const;

  virtual std::shared_ptr<IPropertySource> GetInspected() const;

public: // Events

  virtual void OnActivated();

  virtual void OnDeactivated();

  virtual void OnGui() = 0;

private: // Attributes

  std::shared_ptr<CAssetDocument> m_Document;
};

} // namespace editor
