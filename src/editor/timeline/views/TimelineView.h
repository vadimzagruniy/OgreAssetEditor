#pragma once

#include "TimelineDrag.h"
#include "TimelineKeyHit.h"
#include "TimelineViewRange.h"
#include "editor/timeline/model/TimelineId.h"
#include "editor/timeline/model/TimelineTicks.h"

#include <array>
#include <vector>

struct ImDrawList;
struct ImVec2;

namespace editor
{

class CTimeline;
class CTimelineSelection;
class ITimelinePresenter;
enum class ETimelineTimeFormat;
struct TTimelineTrack;

class CTimelineView
{
public: // Constructor

  CTimelineView();

public: // Interface

  void Draw(ITimelinePresenter & _Presenter);

private: // Implementation

  void DrawToolbar(const CTimeline & _Timeline, ITimelinePresenter & _Presenter);

  void DrawPlaybackButtons(ITimelinePresenter & _Presenter) const;

  void DrawAddTrackMenu(ITimelinePresenter & _Presenter) const;

  void DrawRulerRow(const CTimeline & _Timeline, ITimelinePresenter & _Presenter);

  void DrawRulerTicks(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const CTimeline & _Timeline) const;

  void DrawTrackRow(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track);

  bool DrawTrackHeader(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track);

  void DrawTrackContextMenu(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track);

  void DrawAddKeyButton(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track) const;

  void DrawRenameField(ITimelinePresenter & _Presenter);

  void BeginRename(const TTimelineTrack & _Track);

  void DrawLane(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track);

  void DrawGrid(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const CTimeline & _Timeline) const;

  void DrawKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, const CTimelineSelection & _Selection, bool _IsSummary) const;

  void DrawChildKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, const CTimelineSelection & _Selection) const;

  void DrawClips(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track) const;

  void DrawPlayhead(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max) const;

  void DrawSelectionBox(ITimelinePresenter & _Presenter);

  void CollectKeyHits(const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track);

  TTimelineId FindKeyAt(const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, const ImVec2 & _Point) const;

  void HandleNavigation(const ImVec2 & _Min);

  void HandleLaneMouse(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track, const ImVec2 & _Min, const ImVec2 & _Max);

  void HandleLanePress(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track, const ImVec2 & _Min, const ImVec2 & _Max);

  void HandleLaneDrag(ITimelinePresenter & _Presenter, int _FrameRate);

  void HandleLaneRelease(ITimelinePresenter & _Presenter);

  TTimelineTicks GetMouseTime(const ImVec2 & _Min, int _FrameRate) const;

  void ScrubTo(ITimelinePresenter & _Presenter, float _Offset, int _FrameRate);

private: // Attributes

  CTimelineViewRange           m_Range;
  ETimelineTimeFormat          m_Format;
  TTimelineTicks               m_CurrentTime;
  TTimelineDrag                m_Drag;
  std::vector<TTimelineKeyHit> m_KeyHits;
  TTimelineId                  m_RenamingTrack;
  std::array<char, 256>        m_RenameBuffer;
  bool                         m_IsRenameFocusPending;
  bool                         m_IsSnapping;
  bool                         m_IsFitPending;
};

} // namespace editor
