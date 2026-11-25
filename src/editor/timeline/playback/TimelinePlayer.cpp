#include "TimelinePlayer.h"
#include "ITimelineEvaluator.h"
#include "TimelineEvaluateMode.h"
#include "editor/timeline/model/Timeline.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace editor
{

//
// Constructor
//

CTimelinePlayer::CTimelinePlayer(std::shared_ptr<ITimelineEvaluator> _Evaluator):
  m_Evaluator(std::move(_Evaluator)),
  m_Time(0),
  m_IsPlaying(false),
  m_IsPreviewing(false),
  m_IsLooping(true)
{
  // Empty
}

//
// Interface
//

void CTimelinePlayer::Update(const CTimeline & _Timeline, float _DeltaSeconds)
{
  if (!m_IsPreviewing)
    return;

  if (m_IsPlaying)
  {
    Advance(_Timeline, _DeltaSeconds);
    return;
  }

  m_Evaluator->Evaluate(_Timeline, m_Time, m_Time, ETimelineEvaluateMode::Scrub);
}

void CTimelinePlayer::Play()
{
  BeginPreview();
  m_IsPlaying = true;
}

void CTimelinePlayer::Pause() noexcept
{
  m_IsPlaying = false;
}

void CTimelinePlayer::Stop()
{
  if (m_IsPreviewing)
    m_Evaluator->EndPreview();

  m_IsPreviewing = false;
  m_IsPlaying    = false;
  m_Time         = 0;
}

void CTimelinePlayer::Scrub(TTimelineTicks _Time)
{
  BeginPreview();
  m_Time = std::max<TTimelineTicks>(_Time, 0);
}

bool CTimelinePlayer::IsPlaying() const noexcept
{
  return m_IsPlaying;
}

bool CTimelinePlayer::IsPreviewing() const noexcept
{
  return m_IsPreviewing;
}

bool CTimelinePlayer::IsLooping() const noexcept
{
  return m_IsLooping;
}

void CTimelinePlayer::SetLooping(bool _IsLooping) noexcept
{
  m_IsLooping = _IsLooping;
}

TTimelineTicks CTimelinePlayer::GetTime() const noexcept
{
  return m_Time;
}

//
// Implementation
//

void CTimelinePlayer::BeginPreview()
{
  if (m_IsPreviewing)
    return;

  m_Evaluator->BeginPreview();
  m_IsPreviewing = true;
}

void CTimelinePlayer::Advance(const CTimeline & _Timeline, float _DeltaSeconds)
{
  const TTimelineTicks Duration = _Timeline.GetDuration();
  if (Duration <= 0)
  {
    m_IsPlaying = false;
    return;
  }

  if (m_Time >= Duration)
    m_Time = 0;

  const TTimelineTicks Target = m_Time + std::llround(static_cast<double>(_DeltaSeconds) * static_cast<double>(TimelineTicksPerSecond));

  if (Target < Duration)
  {
    m_Evaluator->Evaluate(_Timeline, m_Time, Target, ETimelineEvaluateMode::Play);
    m_Time = Target;
    return;
  }

  m_Evaluator->Evaluate(_Timeline, m_Time, Duration, ETimelineEvaluateMode::Play);

  if (!m_IsLooping)
  {
    m_Time      = Duration;
    m_IsPlaying = false;
    return;
  }

  m_Time = (Target - Duration) % Duration;
  m_Evaluator->Evaluate(_Timeline, 0, m_Time, ETimelineEvaluateMode::Play);
}

} // namespace editor
