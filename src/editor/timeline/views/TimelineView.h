#pragma once

#include "TimelineViewRange.h"
#include "editor/timeline/model/TimelineTicks.h"

struct ImDrawList;
struct ImVec2;

namespace editor
{

class CTimeline;
class CTimelinePlayer;
enum class ETimelineTimeFormat;
struct TTimelineTrack;

class CTimelineView
{
public: // Constructor

  CTimelineView();

public: // Interface

  void Draw(const CTimeline & _Timeline, CTimelinePlayer & _Player);

private: // Implementation

  void DrawToolbar(const CTimeline & _Timeline, CTimelinePlayer & _Player);

  void DrawPlaybackButtons(CTimelinePlayer & _Player) const;

  void DrawRulerRow(const CTimeline & _Timeline, CTimelinePlayer & _Player);

  void DrawRulerTicks(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const CTimeline & _Timeline) const;

  void DrawTrackRow(const CTimeline & _Timeline, const TTimelineTrack & _Track);

  void DrawLane(const CTimeline & _Timeline, const TTimelineTrack & _Track);

  void DrawGrid(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const CTimeline & _Timeline) const;

  void DrawKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, unsigned int _Color) const;

  void DrawChildKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, unsigned int _Color) const;

  void DrawClips(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track) const;

  void DrawPlayhead(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max) const;

  void HandleLaneInput(const ImVec2 & _Min);

  void ScrubTo(CTimelinePlayer & _Player, float _Offset, int _FrameRate);

private: // Attributes

  CTimelineViewRange  m_Range;
  ETimelineTimeFormat m_Format;
  TTimelineTicks      m_CurrentTime;
  bool                m_IsFitPending;
};

} // namespace editor
