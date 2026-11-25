#pragma once

#include "core/TypeKey.h"

#include <vector>

namespace editor
{

class CAssetDocument;

class IAttachedEditor
{
public: // Destructor

  virtual ~IAttachedEditor();

public: // Interface

  virtual CAssetDocument & GetDocument() const noexcept = 0;

  virtual std::vector<core::TTypeKey> GetToolBoxes() const = 0;

public: // Events

  virtual void OnActivated() = 0;

  virtual void OnDeactivated() = 0;
};

} // namespace editor
