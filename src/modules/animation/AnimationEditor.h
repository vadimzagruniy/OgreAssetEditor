#pragma once

#include "editor/workspaces/IAttachedEditor.h"

#include <memory>

namespace editor
{

class CAnimationDocument;
class CTimelinePlayer;
class CTimelineView;
class ISceneAnimationHost;

class CAnimationEditor final:
  public IAttachedEditor
{
public: // Constructor/Destructor

  CAnimationEditor(std::shared_ptr<CAnimationDocument> _Document, std::weak_ptr<ISceneAnimationHost> _Host);

  ~CAnimationEditor() override;

public: // IAttachedEditor

  CAssetDocument & GetDocument() const noexcept override;

  std::vector<core::TTypeKey> GetToolBoxes() const override;

  void Tick(float _DeltaTime) override;

  void OnActivated() override;

  void OnDeactivated() override;

public: // Interface

  CAnimationDocument & GetAnimation() const noexcept;

  CTimelineView & GetTimelineView() const noexcept;

  CTimelinePlayer & GetPlayer() const noexcept;

private: // Attributes

  std::shared_ptr<CAnimationDocument> m_Document;
  std::unique_ptr<CTimelineView>      m_TimelineView;
  std::unique_ptr<CTimelinePlayer>    m_Player;
};

} // namespace editor
