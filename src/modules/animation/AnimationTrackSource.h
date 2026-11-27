#pragma once

#include "editor/timeline/presenter/ITimelineTrackSource.h"

#include <memory>

namespace editor
{

class CAnimationSlots;

class CAnimationTrackSource final:
  public ITimelineTrackSource
{
public: // Constructor

  explicit CAnimationTrackSource(std::shared_ptr<const CAnimationSlots> _Slots);

public: // ITimelineTrackSource

  std::vector<TTimelineTrackTemplate> GetTrackTemplates(const CTimeline & _Timeline) const override;

  std::vector<TTimelineTrackTemplate> GetTargetTemplates(const CTimeline & _Timeline, const std::string & _Slot) const override;

  std::string GetTrackLabel(const TTimelineTrack & _Track) const override;

  TPropertyValue CaptureValue(const TTimelineTrack & _Track) const override;

  bool IsTargetValid(const TTimelineTrack & _Track) const override;

  bool CanBindToSelection(const TTimelineTrack & _Track) const override;

  void BindToSelection(const TTimelineTrack & _Track) const override;

  std::uint32_t GetTrackColor(const TTimelineTrack & _Track) const override;

  bool HasTrackTarget() const override;

  void SelectTrackTarget(const std::string & _Target) const override;

  void ShowTrackInScene(const TTimelineTrack & _Track) const override;

public: // Events

  void OnTrackAdded(const TTimelineTrack & _Track) const override;

private: // Implementation

  void AddAnimationToScene() const;

private: // Attributes

  std::shared_ptr<const CAnimationSlots> m_Slots;
};

} // namespace editor
