#pragma once

#include "editor/timeline/model/TimelineTicks.h"

namespace editor
{

enum class ETimelineTimeFormat;

class CTimelineViewRange
{
public: // Constructor

  CTimelineViewRange();

public: // Interface

  float ToOffset(TTimelineTicks _Time) const noexcept;

  TTimelineTicks ToTime(float _Offset) const noexcept;

  void Scroll(float _Pixels) noexcept;

  void Zoom(float _Factor, float _AnchorOffset) noexcept;

  void Fit(TTimelineTicks _Duration, float _Width) noexcept;

  float GetZoomLevel() const noexcept;

  void SetZoomLevel(float _Level, float _AnchorOffset) noexcept;

  TTimelineTicks FindGridStep(ETimelineTimeFormat _Format, int _FrameRate, float _MinSpacing) const;

private: // Attributes

  double m_Start;
  double m_PixelsPerSecond;
};

} // namespace editor
