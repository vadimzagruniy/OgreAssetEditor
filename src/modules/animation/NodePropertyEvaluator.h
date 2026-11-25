#pragma once

#include "editor/timeline/tracks/ITimelineTrackEvaluator.h"

#include <memory>
#include <string>
#include <unordered_set>

namespace editor
{

class CAnimationSlots;

class CNodePropertyEvaluator final:
  public ITimelineTrackEvaluator
{
public: // Constructor

  explicit CNodePropertyEvaluator(std::shared_ptr<const CAnimationSlots> _Slots);

public: // ITimelineTrackEvaluator

  void BeginPreview() override;

  void Evaluate(const TTimelineTrack & _Track, TTimelineTicks _From, TTimelineTicks _To, ETimelineEvaluateMode _Mode) override;

  void EndPreview() override;

private: // Attributes

  std::shared_ptr<const CAnimationSlots> m_Slots;
  std::unordered_set<std::string>        m_TouchedNodes;
};

} // namespace editor
