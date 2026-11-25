#pragma once

#include "TimelineViewRange.h"
#include "editor/timeline/model/TimelineTicks.h"

struct ImDrawList;
struct ImVec2;

namespace editor
{

class CTimeline;
enum class ETimelineTimeFormat;
struct TTimelineTrack;

class CTimelineView
{
public: // Constructor

  CTimelineView();

public: // Interface

  void Draw(const CTimeline & _Timeline);

  TTimelineTicks GetCurrentTime() const noexcept;

  void SetCurrentTime(TTimelineTicks _Time) noexcept;

private: // Implementation

  void DrawToolbar(const CTimeline & _Timeline);

  void DrawRulerRow(const CTimeline & _Timeline);

  void DrawRulerTicks(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const CTimeline & _Timeline) const;

  void DrawTrackRow(const CTimeline & _Timeline, const TTimelineTrack & _Track);

  void DrawLane(const CTimeline & _Timeline, const TTimelineTrack & _Track);

  void DrawGrid(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const CTimeline & _Timeline) const;

  void DrawKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, unsigned int _Color) const;

  void DrawChildKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, unsigned int _Color) const;

  void DrawClips(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track) const;

  void DrawPlayhead(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max) const;

  void HandleLaneInput(const ImVec2 & _Min);

  void ScrubTo(float _Offset, int _FrameRate);

private: // Attributes

  CTimelineViewRange  m_Range;
  ETimelineTimeFormat m_Format;
  TTimelineTicks      m_CurrentTime;
  bool                m_IsFitPending;
};

} // namespace editor
