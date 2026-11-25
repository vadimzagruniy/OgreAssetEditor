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

  TPropertyValue CaptureValue(const TTimelineTrack & _Track) const override;

  bool IsTargetValid(const TTimelineTrack & _Track) const override;

  bool CanBindToSelection(const TTimelineTrack & _Track) const override;

  void BindToSelection(const TTimelineTrack & _Track) const override;

private: // Attributes

  std::shared_ptr<const CAnimationSlots> m_Slots;
};

} // namespace editor
