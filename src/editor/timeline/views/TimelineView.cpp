#include "TimelineView.h"
#include "TimelineTime.h"
#include "TimelineTimeFormat.h"
#include "editor/timeline/model/Timeline.h"
#include "editor/timeline/playback/TimelinePlayer.h"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace editor
{

namespace
{

constexpr float TrackColumnWidth = 200.0f;
constexpr float MinGridSpacing   = 80.0f;
constexpr int   MinorTickCount   = 5;
constexpr float KeyRadius        = 5.0f;
constexpr float ClipInset        = 2.0f;
constexpr float ZoomStep         = 1.15f;
constexpr float WheelScrollStep  = 40.0f;
constexpr float PlayheadWidth    = 1.5f;
constexpr ImU32 OutOfRangeShade  = IM_COL32(0, 0, 0, 70);

constexpr std::array<const char *, 2> TimeFormatNames{"Seconds", "Frames"};

float GetRowHeight()
{
  return ImGui::GetFrameHeight();
}

std::string GetTrackLabel(const TTimelineTrack & _Track)
{
  return _Track.Name.empty() ? _Track.TypeId : _Track.Name;
}

std::string GetClipLabel(const TTimelineClip & _Clip)
{
  const auto SlashPosition = _Clip.Source.find_last_of("/\\");
  return SlashPosition == std::string::npos ? _Clip.Source : _Clip.Source.substr(SlashPosition + 1);
}

TTimelineTicks FloorToStep(TTimelineTicks _Time, TTimelineTicks _Step)
{
  return _Time / _Step * _Step;
}

} // namespace

//
// Constructor
//

CTimelineView::CTimelineView():
  m_Format(ETimelineTimeFormat::Seconds),
  m_CurrentTime(0),
  m_IsFitPending(true)
{
  // Empty
}

//
// Interface
//

void CTimelineView::Draw(const CTimeline & _Timeline, CTimelinePlayer & _Player)
{
  m_CurrentTime = _Player.GetTime();

  DrawToolbar(_Timeline, _Player);

  const ImGuiTableFlags TableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg;
  if (!ImGui::BeginTable("##TimelineTracks", 2, TableFlags))
    return;

  ImGui::TableSetupScrollFreeze(0, 1);
  ImGui::TableSetupColumn("Tracks", ImGuiTableColumnFlags_WidthFixed, TrackColumnWidth);
  ImGui::TableSetupColumn("Lanes", ImGuiTableColumnFlags_WidthStretch);

  DrawRulerRow(_Timeline, _Player);

  for (const auto & Track : _Timeline.GetTracks())
    DrawTrackRow(_Timeline, Track);

  if (_Timeline.GetTracks().empty())
  {
    ImGui::TableNextRow(ImGuiTableRowFlags_None, GetRowHeight());
    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("No tracks");
  }

  ImGui::EndTable();
}

//
// Implementation
//

void CTimelineView::DrawToolbar(const CTimeline & _Timeline, CTimelinePlayer & _Player)
{
  DrawPlaybackButtons(_Player);

  int FormatIndex = static_cast<int>(m_Format);

  ImGui::SameLine();

  ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.0f);
  if (ImGui::Combo("##TimeFormat", &FormatIndex, TimeFormatNames.data(), static_cast<int>(TimeFormatNames.size())))
    m_Format = static_cast<ETimelineTimeFormat>(FormatIndex);

  ImGui::SameLine();
  if (ImGui::Button("Fit"))
    m_IsFitPending = true;

  const auto FrameRate = _Timeline.GetFrameRate();
  const auto TimeText  = FormatTimelineTime(m_CurrentTime, m_Format, FrameRate) + " / " + FormatTimelineTime(_Timeline.GetDuration(), m_Format, FrameRate);

  ImGui::SameLine();
  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(TimeText.c_str());
}

void CTimelineView::DrawPlaybackButtons(CTimelinePlayer & _Player) const
{
  if (ImGui::Button(_Player.IsPlaying() ? "Pause" : "Play"))
  {
    if (_Player.IsPlaying())
      _Player.Pause();
    else
      _Player.Play();
  }

  ImGui::SameLine();
  ImGui::BeginDisabled(!_Player.IsPreviewing());
  if (ImGui::Button("Stop"))
    _Player.Stop();
  ImGui::EndDisabled();

  bool IsLooping = _Player.IsLooping();

  ImGui::SameLine();
  if (ImGui::Checkbox("Loop", &IsLooping))
    _Player.SetLooping(IsLooping);
}

void CTimelineView::DrawRulerRow(const CTimeline & _Timeline, CTimelinePlayer & _Player)
{
  const float RowHeight = GetRowHeight();

  ImGui::TableNextRow(ImGuiTableRowFlags_Headers, RowHeight);
  ImGui::TableSetColumnIndex(1);

  const ImVec2 Min   = ImGui::GetCursorScreenPos();
  const float  Width = std::max(ImGui::GetContentRegionAvail().x, 1.0f);
  const ImVec2 Max(Min.x + Width, Min.y + RowHeight);

  if (m_IsFitPending)
  {
    m_Range.Fit(_Timeline.GetDuration(), Width);
    m_IsFitPending = false;
  }

  ImGui::InvisibleButton("##Ruler", ImVec2(Width, RowHeight));
  if (ImGui::IsItemActive())
    ScrubTo(_Player, ImGui::GetIO().MousePos.x - Min.x, _Timeline.GetFrameRate());

  HandleLaneInput(Min);

  auto & DrawList = *ImGui::GetWindowDrawList();
  DrawList.AddRectFilled(Min, Max, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
  DrawRulerTicks(DrawList, Min, Max, _Timeline);
  DrawPlayhead(DrawList, Min, Max);
}

void CTimelineView::DrawRulerTicks(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const CTimeline & _Timeline) const
{
  const auto  FrameRate  = _Timeline.GetFrameRate();
  const auto  Step       = m_Range.FindGridStep(m_Format, FrameRate, MinGridSpacing);
  const auto  MinorStep  = std::max<TTimelineTicks>(Step / MinorTickCount, 1);
  const float Height     = _Max.y - _Min.y;
  const ImU32 TickColor  = ImGui::GetColorU32(ImGuiCol_TextDisabled);
  const ImU32 LabelColor = ImGui::GetColorU32(ImGuiCol_Text);

  for (TTimelineTicks Time = FloorToStep(m_Range.ToTime(0.0f), MinorStep); ; Time += MinorStep)
  {
    const float PositionX = _Min.x + m_Range.ToOffset(Time);
    if (PositionX > _Max.x)
      break;

    const bool  IsMajor    = Time % Step == 0;
    const float TickHeight = IsMajor ? Height * 0.5f : Height * 0.2f;

    _DrawList.AddLine(ImVec2(PositionX, _Max.y - TickHeight), ImVec2(PositionX, _Max.y), TickColor);

    if (IsMajor)
      _DrawList.AddText(ImVec2(PositionX + 3.0f, _Min.y), LabelColor, FormatTimelineTime(Time, m_Format, FrameRate).c_str());
  }

  const float DurationX = _Min.x + m_Range.ToOffset(_Timeline.GetDuration());
  if (DurationX < _Max.x)
    _DrawList.AddRectFilled(ImVec2(std::max(DurationX, _Min.x), _Min.y), _Max, OutOfRangeShade);
}

void CTimelineView::DrawTrackRow(const CTimeline & _Timeline, const TTimelineTrack & _Track)
{
  ImGui::TableNextRow(ImGuiTableRowFlags_None, GetRowHeight());
  ImGui::TableSetColumnIndex(0);
  ImGui::PushID(static_cast<int>(_Track.Id));

  const bool         HasChildren = !_Track.Children.empty();
  ImGuiTreeNodeFlags Flags       = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;

  if (!HasChildren)
    Flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

  ImGui::AlignTextToFramePadding();
  const bool IsOpen = ImGui::TreeNodeEx("##Track", Flags, "%s", GetTrackLabel(_Track).c_str());

  ImGui::TableSetColumnIndex(1);
  DrawLane(_Timeline, _Track);

  if (HasChildren && IsOpen)
  {
    for (const auto & Child : _Track.Children)
      DrawTrackRow(_Timeline, Child);

    ImGui::TreePop();
  }

  ImGui::PopID();
}

void CTimelineView::DrawLane(const CTimeline & _Timeline, const TTimelineTrack & _Track)
{
  const float  RowHeight = GetRowHeight();
  const ImVec2 Min       = ImGui::GetCursorScreenPos();
  const float  Width     = std::max(ImGui::GetContentRegionAvail().x, 1.0f);
  const ImVec2 Max(Min.x + Width, Min.y + RowHeight);

  ImGui::InvisibleButton("##Lane", ImVec2(Width, RowHeight));
  HandleLaneInput(Min);

  auto & DrawList = *ImGui::GetWindowDrawList();

  DrawGrid(DrawList, Min, Max, _Timeline);
  DrawClips(DrawList, Min, Max, _Track);
  DrawKeys(DrawList, Min, Max, _Track, ImGui::GetColorU32(ImGuiCol_Text));
  DrawChildKeys(DrawList, Min, Max, _Track, ImGui::GetColorU32(ImGuiCol_TextDisabled));
  DrawPlayhead(DrawList, Min, Max);
}

void CTimelineView::DrawGrid(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const CTimeline & _Timeline) const
{
  const auto  Step      = m_Range.FindGridStep(m_Format, _Timeline.GetFrameRate(), MinGridSpacing);
  const ImU32 GridColor = ImGui::GetColorU32(ImGuiCol_TableBorderLight);

  for (TTimelineTicks Time = FloorToStep(m_Range.ToTime(0.0f), Step); ; Time += Step)
  {
    const float PositionX = _Min.x + m_Range.ToOffset(Time);
    if (PositionX > _Max.x)
      break;

    _DrawList.AddLine(ImVec2(PositionX, _Min.y), ImVec2(PositionX, _Max.y), GridColor);
  }

  const float DurationX = _Min.x + m_Range.ToOffset(_Timeline.GetDuration());
  if (DurationX < _Max.x)
    _DrawList.AddRectFilled(ImVec2(std::max(DurationX, _Min.x), _Min.y), _Max, OutOfRangeShade);
}

void CTimelineView::DrawKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, unsigned int _Color) const
{
  const float CenterY = (_Min.y + _Max.y) * 0.5f;

  for (const auto & Key : _Track.Keys)
  {
    const float PositionX = _Min.x + m_Range.ToOffset(Key.Time);
    if (PositionX < _Min.x - KeyRadius || PositionX > _Max.x + KeyRadius)
      continue;

    if (Key.Interpolation == ETimelineInterpolation::Step)
    {
      _DrawList.AddRectFilled(ImVec2(PositionX - KeyRadius * 0.8f, CenterY - KeyRadius * 0.8f), ImVec2(PositionX + KeyRadius * 0.8f, CenterY + KeyRadius * 0.8f), _Color);
      continue;
    }

    _DrawList.AddQuadFilled(ImVec2(PositionX, CenterY - KeyRadius), ImVec2(PositionX + KeyRadius, CenterY), ImVec2(PositionX, CenterY + KeyRadius), ImVec2(PositionX - KeyRadius, CenterY), _Color);
  }
}

void CTimelineView::DrawChildKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, unsigned int _Color) const
{
  for (const auto & Child : _Track.Children)
  {
    DrawKeys(_DrawList, _Min, _Max, Child, _Color);
    DrawChildKeys(_DrawList, _Min, _Max, Child, _Color);
  }
}

void CTimelineView::DrawClips(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track) const
{
  const ImU32 FillColor   = ImGui::GetColorU32(ImGuiCol_Button);
  const ImU32 BorderColor = ImGui::GetColorU32(ImGuiCol_Border);
  const ImU32 TextColor   = ImGui::GetColorU32(ImGuiCol_Text);

  for (const auto & Clip : _Track.Clips)
  {
    const ImVec2 ClipMin(_Min.x + m_Range.ToOffset(Clip.Start), _Min.y + ClipInset);
    const ImVec2 ClipMax(_Min.x + m_Range.ToOffset(Clip.Start + Clip.Length), _Max.y - ClipInset);

    if (ClipMax.x < _Min.x || ClipMin.x > _Max.x)
      continue;

    _DrawList.AddRectFilled(ClipMin, ClipMax, FillColor, 3.0f);
    _DrawList.AddRect(ClipMin, ClipMax, BorderColor, 3.0f);

    _DrawList.PushClipRect(ImVec2(std::max(ClipMin.x, _Min.x), ClipMin.y), ImVec2(std::min(ClipMax.x, _Max.x), ClipMax.y), true);
    _DrawList.AddText(ImVec2(ClipMin.x + 4.0f, ClipMin.y), TextColor, GetClipLabel(Clip).c_str());
    _DrawList.PopClipRect();
  }
}

void CTimelineView::DrawPlayhead(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max) const
{
  const float PositionX = _Min.x + m_Range.ToOffset(m_CurrentTime);
  if (PositionX < _Min.x || PositionX > _Max.x)
    return;

  _DrawList.AddLine(ImVec2(PositionX, _Min.y), ImVec2(PositionX, _Max.y), ImGui::GetColorU32(ImGuiCol_SliderGrab), PlayheadWidth);
}

void CTimelineView::HandleLaneInput(const ImVec2 & _Min)
{
  if (!ImGui::IsItemHovered())
    return;

  const auto & Io = ImGui::GetIO();

  if (Io.KeyCtrl || Io.KeyShift)
    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);

  if (Io.KeyCtrl && Io.MouseWheel != 0.0f)
    m_Range.Zoom(std::pow(ZoomStep, Io.MouseWheel), Io.MousePos.x - _Min.x);
  else
  if (Io.KeyShift && Io.MouseWheel != 0.0f)
    m_Range.Scroll(-Io.MouseWheel * WheelScrollStep);

  if (Io.MouseWheelH != 0.0f)
    m_Range.Scroll(-Io.MouseWheelH * WheelScrollStep);

  if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
    m_Range.Scroll(-Io.MouseDelta.x);
}

void CTimelineView::ScrubTo(CTimelinePlayer & _Player, float _Offset, int _FrameRate)
{
  m_CurrentTime = std::max<TTimelineTicks>(SnapToFrame(m_Range.ToTime(_Offset), _FrameRate), 0);
  _Player.Scrub(m_CurrentTime);
}

} // namespace editor
