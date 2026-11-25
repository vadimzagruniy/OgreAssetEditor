#pragma once

#include "editor/workspaces/IAttachedEditor.h"

#include <memory>

namespace editor
{

class CAnimationDocument;
class CTimelineView;

class CAnimationEditor final:
  public IAttachedEditor
{
public: // Constructor/Destructor

  explicit CAnimationEditor(std::shared_ptr<CAnimationDocument> _Document);

  ~CAnimationEditor() override;

public: // IAttachedEditor

  CAssetDocument & GetDocument() const noexcept override;

  std::vector<core::TTypeKey> GetToolBoxes() const override;

  void OnActivated() override;

  void OnDeactivated() override;

public: // Interface

  CAnimationDocument & GetAnimation() const noexcept;

  CTimelineView & GetTimelineView() const noexcept;

private: // Attributes

  std::shared_ptr<CAnimationDocument> m_Document;
  std::unique_ptr<CTimelineView>      m_TimelineView;
};

} // namespace editor
