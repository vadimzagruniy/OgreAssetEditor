#pragma once

#include "editor/timeline/presenter/ITimelineTrackSource.h"

#include <memory>

namespace editor
{

class ISceneAnimationHost;

class CAnimationTrackSource final:
  public ITimelineTrackSource
{
public: // Constructor

  explicit CAnimationTrackSource(std::weak_ptr<ISceneAnimationHost> _Host);

public: // ITimelineTrackSource

  std::vector<TTimelineTrackTemplate> GetTrackTemplates(const CTimeline & _Timeline) const override;

  TPropertyValue CaptureValue(const TTimelineTrack & _Track) const override;

private: // Attributes

  std::weak_ptr<ISceneAnimationHost> m_Host;
};

} // namespace editor
