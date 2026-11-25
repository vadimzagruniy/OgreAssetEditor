#pragma once

#include "editor/timeline/playback/ITimelineEvaluator.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace editor
{

class ISceneAnimationHost;
struct TTimelineTrack;

class CAnimationPreview final:
  public ITimelineEvaluator
{
public: // Constructor

  explicit CAnimationPreview(std::weak_ptr<ISceneAnimationHost> _Host);

public: // ITimelineEvaluator

  void BeginPreview() override;

  void Evaluate(const CTimeline & _Timeline, TTimelineTicks _From, TTimelineTicks _To, ETimelineEvaluateMode _Mode) override;

  void EndPreview() override;

private: // Implementation

  void EvaluateTracks(const std::vector<TTimelineTrack> & _Tracks, TTimelineTicks _Time, ISceneAnimationHost & _Host);

  void ApplyTransform(const TTimelineTrack & _Track, TTimelineTicks _Time, ISceneAnimationHost & _Host);

private: // Attributes

  std::weak_ptr<ISceneAnimationHost> m_Host;
  std::unordered_set<std::string>    m_TouchedNodes;
};

} // namespace editor
