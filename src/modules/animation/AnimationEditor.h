#pragma once

#include "editor/workspaces/IAttachedEditor.h"

#include <memory>

namespace editor
{

namespace core
{
class CActionQueue;
} // namespace core

class CAnimationDocument;
class CAnimationSlots;
class CTimelinePresenter;
class CTimelineView;
class ISceneAnimationHost;

class CAnimationEditor final:
  public IAttachedEditor
{
public: // Constructor/Destructor

  CAnimationEditor(std::shared_ptr<CAnimationDocument> _Document, std::weak_ptr<ISceneAnimationHost> _Host, std::shared_ptr<core::CActionQueue> _ActionQueue);

  ~CAnimationEditor() override;

public: // IAttachedEditor

  CAssetDocument & GetDocument() const noexcept override;

  std::vector<core::TTypeKey> GetToolBoxes() const override;

  std::shared_ptr<IPropertySource> GetInspected() const override;

  void Tick(float _DeltaTime) override;

  void OnActivated() override;

  void OnDeactivated() override;

public: // Interface

  CAnimationDocument & GetAnimation() const noexcept;

  CTimelineView & GetTimelineView() const noexcept;

  CTimelinePresenter & GetPresenter() const noexcept;

  bool IsInScene() const;

  void AddToScene() const;

  void RemoveFromScene() const;

private: // Attributes

  std::shared_ptr<CAnimationDocument>    m_Document;
  std::shared_ptr<const CAnimationSlots> m_Slots;
  std::unique_ptr<CTimelineView>         m_TimelineView;
  std::unique_ptr<CTimelinePresenter>    m_Presenter;
};

} // namespace editor
