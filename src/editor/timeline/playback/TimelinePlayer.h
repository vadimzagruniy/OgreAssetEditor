#pragma once

#include "editor/timeline/model/TimelineTicks.h"

#include <memory>

namespace editor
{

class CTimeline;
class ITimelineEvaluator;

class CTimelinePlayer
{
public: // Constructor

  explicit CTimelinePlayer(std::shared_ptr<ITimelineEvaluator> _Evaluator);

public: // Interface

  void Update(const CTimeline & _Timeline, float _DeltaSeconds);

  void Play();

  void Pause() noexcept;

  void Stop();

  void Scrub(TTimelineTicks _Time);

  bool IsPlaying() const noexcept;

  bool IsPreviewing() const noexcept;

  bool IsLooping() const noexcept;

  void SetLooping(bool _IsLooping) noexcept;

  TTimelineTicks GetTime() const noexcept;

private: // Implementation

  void BeginPreview();

  void Advance(const CTimeline & _Timeline, float _DeltaSeconds);

private: // Attributes

  std::shared_ptr<ITimelineEvaluator> m_Evaluator;
  TTimelineTicks                      m_Time;
  bool                                m_IsPlaying;
  bool                                m_IsPreviewing;
  bool                                m_IsLooping;
};

} // namespace editor
