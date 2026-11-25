#pragma once

#include "editor/timeline/tracks/ITimelineTrackEvaluator.h"

#include <memory>
#include <string>
#include <unordered_set>

namespace editor
{

class ISceneAnimationHost;

class CNodePropertyEvaluator final:
  public ITimelineTrackEvaluator
{
public: // Constructor

  explicit CNodePropertyEvaluator(std::weak_ptr<ISceneAnimationHost> _Host);

public: // ITimelineTrackEvaluator

  void BeginPreview() override;

  void Evaluate(const TTimelineTrack & _Track, TTimelineTicks _From, TTimelineTicks _To, ETimelineEvaluateMode _Mode) override;

  void EndPreview() override;

private: // Attributes

  std::weak_ptr<ISceneAnimationHost> m_Host;
  std::unordered_set<std::string>    m_TouchedNodes;
};

} // namespace editor
