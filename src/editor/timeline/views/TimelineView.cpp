#include "TimelineView.h"
#include "TimelineTime.h"
#include "TimelineTimeFormat.h"
#include "editor/timeline/model/Timeline.h"
#include "editor/timeline/presenter/ITimelinePresenter.h"
#include "editor/timeline/presenter/TimelineSelectMode.h"
#include "editor/timeline/presenter/TimelineSelection.h"
#include "editor/timeline/tracks/TimelineItemKind.h"

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
constexpr float KeyHitRadius     = 7.0f;
constexpr float ClipInset        = 2.0f;
constexpr float ZoomStep         = 1.15f;
constexpr float WheelScrollStep  = 40.0f;
constexpr float PlayheadWidth    = 1.5f;
constexpr float MinBoxSize       = 3.0f;
constexpr ImU32 OutOfRangeShade  = IM_COL32(0, 0, 0, 70);
constexpr ImU32 MissingTarget    = IM_COL32(230, 150, 60, 255);

constexpr std::array<const char *, 2> TimeFormatNames{"Seconds", "Frames"};

float GetRowHeight()
{
  return ImGui::GetFrameHeight();
}

float GetCenterY(const ImVec2 & _Min, const ImVec2 & _Max)
{
  return (_Min.y + _Max.y) * 0.5f;
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

TTimelineTicks SnapDeltaToFrame(TTimelineTicks _Delta, int _FrameRate)
{
  const double FrameTicks = static_cast<double>(TimelineTicksPerSecond) / static_cast<double>(std::max(_FrameRate, 1));
  return static_cast<TTimelineTicks>(std::llround(std::round(static_cast<double>(_Delta) / FrameTicks) * FrameTicks));
}

} // namespace

//
// Constructor
//

CTimelineView::CTimelineView():
  m_Format(ETimelineTimeFormat::Seconds),
  m_CurrentTime(0),
  m_RenamingTrack(InvalidTimelineId),
  m_RenameBuffer{},
  m_IsRenameFocusPending(false),
  m_IsSnapping(true),
  m_IsFitPending(true)
{
  // Empty
}

//
// Interface
//

void CTimelineView::Draw(ITimelinePresenter & _Presenter)
{
  const auto & Timeline = _Presenter.GetTimeline();

  m_CurrentTime = _Presenter.GetTime();
  m_KeyHits.clear();

  DrawToolbar(Timeline, _Presenter);

  const ImGuiTableFlags TableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg;
  if (!ImGui::BeginTable("##TimelineTracks", 2, TableFlags))
    return;

  ImGui::TableSetupScrollFreeze(0, 1);
  ImGui::TableSetupColumn("Tracks", ImGuiTableColumnFlags_WidthFixed, TrackColumnWidth);
  ImGui::TableSetupColumn("Lanes", ImGuiTableColumnFlags_WidthStretch);

  DrawRulerRow(Timeline, _Presenter);

  for (const auto & Track : Timeline.GetTracks())
    DrawTrackRow(_Presenter, Track);

  if (Timeline.GetTracks().empty())
  {
    ImGui::TableNextRow(ImGuiTableRowFlags_None, GetRowHeight());
    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("No tracks");
  }

  DrawSelectionBox(_Presenter);

  ImGui::EndTable();

  if (ImGui::Shortcut(ImGuiKey_Delete))
    _Presenter.DeleteSelection();
}

//
// Implementation
//

void CTimelineView::DrawToolbar(const CTimeline & _Timeline, ITimelinePresenter & _Presenter)
{
  DrawPlaybackButtons(_Presenter);

  int FormatIndex = static_cast<int>(m_Format);

  ImGui::SameLine();

  ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.0f);
  if (ImGui::Combo("##TimeFormat", &FormatIndex, TimeFormatNames.data(), static_cast<int>(TimeFormatNames.size())))
    m_Format = static_cast<ETimelineTimeFormat>(FormatIndex);

  ImGui::SameLine();
  ImGui::Checkbox("Snap", &m_IsSnapping);

  ImGui::SameLine();
  if (ImGui::Button("Fit"))
    m_IsFitPending = true;

  ImGui::SameLine();
  DrawAddTrackMenu(_Presenter);

  const auto FrameRate = _Timeline.GetFrameRate();
  const auto TimeText  = FormatTimelineTime(m_CurrentTime, m_Format, FrameRate) + " / " + FormatTimelineTime(_Timeline.GetDuration(), m_Format, FrameRate);

  ImGui::SameLine();
  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(TimeText.c_str());
}

void CTimelineView::DrawPlaybackButtons(ITimelinePresenter & _Presenter) const
{
  if (ImGui::Button(_Presenter.IsPlaying() ? "Pause" : "Play"))
  {
    if (_Presenter.IsPlaying())
      _Presenter.Pause();
    else
      _Presenter.Play();
  }

  ImGui::SameLine();
  ImGui::BeginDisabled(!_Presenter.IsPreviewing());
  if (ImGui::Button("Stop"))
    _Presenter.Stop();
  ImGui::EndDisabled();

  bool IsLooping = _Presenter.IsLooping();

  ImGui::SameLine();
  if (ImGui::Checkbox("Loop", &IsLooping))
    _Presenter.SetLooping(IsLooping);
}

void CTimelineView::DrawAddTrackMenu(ITimelinePresenter & _Presenter) const
{
  if (ImGui::Button("Add Track"))
    ImGui::OpenPopup("##AddTrackMenu");

  if (!ImGui::BeginPopup("##AddTrackMenu"))
    return;

  const auto Templates = _Presenter.GetTrackTemplates();

  for (const auto & Template : Templates)
  {
    if (ImGui::MenuItem(Template.Label.c_str()))
      _Presenter.AddTrack(Template.Track);
  }

  if (Templates.empty())
    ImGui::TextDisabled("Select a scene node to add its tracks");

  ImGui::Separator();

  if (ImGui::MenuItem("Group"))
  {
    TTimelineTrack Group;
    Group.TypeId = TimelineGroupType;
    Group.Name   = "Group";

    _Presenter.AddTrack(Group);
  }

  ImGui::EndPopup();
}

void CTimelineView::DrawRulerRow(const CTimeline & _Timeline, ITimelinePresenter & _Presenter)
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
    ScrubTo(_Presenter, ImGui::GetIO().MousePos.x - Min.x, _Timeline.GetFrameRate());

  HandleNavigation(Min);

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

void CTimelineView::DrawTrackRow(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track)
{
  ImGui::TableNextRow(ImGuiTableRowFlags_None, GetRowHeight());
  ImGui::TableSetColumnIndex(0);
  ImGui::PushID(static_cast<int>(_Track.Id));

  const bool IsOpen = DrawTrackHeader(_Presenter, _Track);

  ImGui::TableSetColumnIndex(1);
  DrawLane(_Presenter, _Track);

  if (!_Track.Children.empty() && IsOpen)
  {
    for (const auto & Child : _Track.Children)
      DrawTrackRow(_Presenter, Child);

    ImGui::TreePop();
  }

  ImGui::PopID();
}

bool CTimelineView::DrawTrackHeader(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track)
{
  const bool         HasChildren = !_Track.Children.empty();
  ImGuiTreeNodeFlags Flags       = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap;

  if (!HasChildren)
    Flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

  if (_Presenter.GetSelection().GetTrack() == _Track.Id)
    Flags |= ImGuiTreeNodeFlags_Selected;

  const bool        IsRenaming = m_RenamingTrack == _Track.Id;
  const std::string Label      = IsRenaming ? std::string() : GetTrackLabel(_Track);

  const bool IsMissing = !_Presenter.IsTargetValid(_Track);

  if (IsMissing)
    ImGui::PushStyleColor(ImGuiCol_Text, MissingTarget);

  ImGui::AlignTextToFramePadding();
  const bool IsOpen = ImGui::TreeNodeEx("##Track", Flags, "%s", Label.c_str());

  if (IsMissing)
  {
    ImGui::PopStyleColor();
    ImGui::SetItemTooltip("Target node '%s' is not in the scene", _Track.Target.Slot.c_str());
  }

  if ((ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemToggledOpen())
    _Presenter.SelectTrack(_Track.Id);

  DrawTrackContextMenu(_Presenter, _Track);

  if (IsRenaming)
  {
    ImGui::SameLine();
    DrawRenameField(_Presenter);
    return IsOpen;
  }

  if (_Presenter.GetItemKind(_Track) == ETimelineItemKind::Keys)
    DrawAddKeyButton(_Presenter, _Track);

  return IsOpen;
}

void CTimelineView::DrawTrackContextMenu(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track)
{
  if (!ImGui::BeginPopupContextItem("##TrackMenu"))
    return;

  if (ImGui::MenuItem("Rename"))
    BeginRename(_Track);

  if (ImGui::MenuItem("Bind to Selected Node", nullptr, false, _Presenter.CanBindToSelection(_Track)))
    _Presenter.BindToSelection(_Track);

  if (ImGui::MenuItem("Delete", "Del"))
    _Presenter.DeleteTrack(_Track.Id);

  ImGui::EndPopup();
}

void CTimelineView::DrawAddKeyButton(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track) const
{
  const float ButtonWidth = ImGui::CalcTextSize("+").x + ImGui::GetStyle().FramePadding.x * 2.0f;

  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(ImGui::GetContentRegionAvail().x - ButtonWidth, 0.0f));

  ImGui::BeginDisabled(!_Presenter.CanAddKey(_Track.Id));
  if (ImGui::SmallButton("+"))
    _Presenter.AddKey(_Track.Id, m_CurrentTime);
  ImGui::EndDisabled();

  ImGui::SetItemTooltip("Add key at playhead");
}

void CTimelineView::DrawRenameField(ITimelinePresenter & _Presenter)
{
  if (m_IsRenameFocusPending)
    ImGui::SetKeyboardFocusHere();

  ImGui::SetNextItemWidth(-1.0f);

  const ImGuiInputTextFlags InputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

  if (ImGui::InputText("##Rename", m_RenameBuffer.data(), m_RenameBuffer.size(), InputFlags))
  {
    const std::string NewName = m_RenameBuffer.data();

    if (!NewName.empty())
      _Presenter.RenameTrack(m_RenamingTrack, NewName);

    m_RenamingTrack = InvalidTimelineId;
    return;
  }

  if (!m_IsRenameFocusPending && !ImGui::IsItemActive())
    m_RenamingTrack = InvalidTimelineId;

  m_IsRenameFocusPending = false;
}

void CTimelineView::BeginRename(const TTimelineTrack & _Track)
{
  const auto Label  = GetTrackLabel(_Track);
  const auto Length = std::min(Label.size(), m_RenameBuffer.size() - 1);

  m_RenameBuffer.fill('\0');
  std::copy_n(Label.begin(), Length, m_RenameBuffer.begin());

  m_RenamingTrack        = _Track.Id;
  m_IsRenameFocusPending = true;
}

void CTimelineView::DrawLane(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track)
{
  const auto & Timeline  = _Presenter.GetTimeline();
  const auto & Selection = _Presenter.GetSelection();
  const float  RowHeight = GetRowHeight();
  const ImVec2 Min       = ImGui::GetCursorScreenPos();
  const float  Width     = std::max(ImGui::GetContentRegionAvail().x, 1.0f);
  const ImVec2 Max(Min.x + Width, Min.y + RowHeight);

  ImGui::InvisibleButton("##Lane", ImVec2(Width, RowHeight));
  HandleNavigation(Min);
  HandleLaneMouse(_Presenter, _Track, Min, Max);
  CollectKeyHits(Min, Max, _Track);

  auto & DrawList = *ImGui::GetWindowDrawList();

  DrawGrid(DrawList, Min, Max, Timeline);
  DrawClips(DrawList, Min, Max, _Track);
  DrawKeys(DrawList, Min, Max, _Track, Selection, false);
  DrawChildKeys(DrawList, Min, Max, _Track, Selection);
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

void CTimelineView::DrawKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, const CTimelineSelection & _Selection, bool _IsSummary) const
{
  const float CenterY       = GetCenterY(_Min, _Max);
  const ImU32 NormalColor   = ImGui::GetColorU32(_IsSummary ? ImGuiCol_TextDisabled : ImGuiCol_Text);
  const ImU32 SelectedColor = ImGui::GetColorU32(ImGuiCol_SliderGrab);

  for (const auto & Key : _Track.Keys)
  {
    const float PositionX = _Min.x + m_Range.ToOffset(Key.Time);
    if (PositionX < _Min.x - KeyRadius || PositionX > _Max.x + KeyRadius)
      continue;

    const ImU32 Color = !_IsSummary && _Selection.IsSelected(Key.Id) ? SelectedColor : NormalColor;

    if (Key.Interpolation == ETimelineInterpolation::Step)
    {
      _DrawList.AddRectFilled(ImVec2(PositionX - KeyRadius * 0.8f, CenterY - KeyRadius * 0.8f), ImVec2(PositionX + KeyRadius * 0.8f, CenterY + KeyRadius * 0.8f), Color);
      continue;
    }

    _DrawList.AddQuadFilled(ImVec2(PositionX, CenterY - KeyRadius), ImVec2(PositionX + KeyRadius, CenterY), ImVec2(PositionX, CenterY + KeyRadius), ImVec2(PositionX - KeyRadius, CenterY), Color);
  }
}

void CTimelineView::DrawChildKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, const CTimelineSelection & _Selection) const
{
  for (const auto & Child : _Track.Children)
  {
    DrawKeys(_DrawList, _Min, _Max, Child, _Selection, true);
    DrawChildKeys(_DrawList, _Min, _Max, Child, _Selection);
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

void CTimelineView::DrawSelectionBox(ITimelinePresenter & _Presenter)
{
  if (m_Drag.Mode != ETimelineDragMode::BoxSelect)
    return;

  const ImVec2 Mouse = ImGui::GetIO().MousePos;
  const ImVec2 BoxMin(std::min(m_Drag.StartX, Mouse.x), std::min(m_Drag.StartY, Mouse.y));
  const ImVec2 BoxMax(std::max(m_Drag.StartX, Mouse.x), std::max(m_Drag.StartY, Mouse.y));
  const bool   IsBox = BoxMax.x - BoxMin.x >= MinBoxSize || BoxMax.y - BoxMin.y >= MinBoxSize;

  if (!m_Drag.IsPending)
  {
    if (IsBox)
    {
      const ImVec2 WindowMin = ImGui::GetWindowPos();
      const ImVec2 WindowMax(WindowMin.x + ImGui::GetWindowWidth(), WindowMin.y + ImGui::GetWindowHeight());
      auto &       DrawList  = *ImGui::GetWindowDrawList();

      DrawList.PushClipRect(WindowMin, WindowMax, false);
      DrawList.AddRectFilled(BoxMin, BoxMax, ImGui::GetColorU32(ImGuiCol_SliderGrab, 0.15f));
      DrawList.AddRect(BoxMin, BoxMax, ImGui::GetColorU32(ImGuiCol_SliderGrab));
      DrawList.PopClipRect();
    }

    return;
  }

  if (IsBox)
  {
    std::vector<TTimelineId> ItemIds;

    for (const auto & Hit : m_KeyHits)
    {
      if (Hit.PositionX >= BoxMin.x && Hit.PositionX <= BoxMax.x && Hit.PositionY >= BoxMin.y && Hit.PositionY <= BoxMax.y)
        ItemIds.push_back(Hit.Id);
    }

    _Presenter.SelectItems(ItemIds, m_Drag.IsAdding);
  }

  m_Drag = TTimelineDrag();
}

void CTimelineView::CollectKeyHits(const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track)
{
  const float CenterY = GetCenterY(_Min, _Max);

  for (const auto & Key : _Track.Keys)
    m_KeyHits.push_back({Key.Id, _Min.x + m_Range.ToOffset(Key.Time), CenterY});
}

TTimelineId CTimelineView::FindKeyAt(const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, const ImVec2 & _Point) const
{
  if (std::abs(_Point.y - GetCenterY(_Min, _Max)) > KeyHitRadius)
    return InvalidTimelineId;

  TTimelineId BestId       = InvalidTimelineId;
  float       BestDistance = KeyHitRadius;

  for (const auto & Key : _Track.Keys)
  {
    const float Distance = std::abs(_Point.x - (_Min.x + m_Range.ToOffset(Key.Time)));
    if (Distance > BestDistance)
      continue;

    BestId       = Key.Id;
    BestDistance = Distance;
  }

  return BestId;
}

void CTimelineView::HandleNavigation(const ImVec2 & _Min)
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

void CTimelineView::HandleLaneMouse(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track, const ImVec2 & _Min, const ImVec2 & _Max)
{
  const int FrameRate = _Presenter.GetTimeline().GetFrameRate();

  const bool IsDoubleClick = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

  if (IsDoubleClick && FindKeyAt(_Min, _Max, _Track, ImGui::GetIO().MousePos) == InvalidTimelineId)
  {
    _Presenter.AddKey(_Track.Id, GetMouseTime(_Min, FrameRate));
    m_Drag = TTimelineDrag();
    return;
  }

  if (ImGui::IsItemActivated())
    HandleLanePress(_Presenter, _Track, _Min, _Max);

  if (ImGui::IsItemActive())
    HandleLaneDrag(_Presenter, FrameRate);

  if (ImGui::IsItemDeactivated())
    HandleLaneRelease(_Presenter);
}

void CTimelineView::HandleLanePress(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track, const ImVec2 & _Min, const ImVec2 & _Max)
{
  const auto &      Io    = ImGui::GetIO();
  const TTimelineId KeyId = FindKeyAt(_Min, _Max, _Track, Io.MousePos);

  m_Drag          = TTimelineDrag();
  m_Drag.StartX   = Io.MousePos.x;
  m_Drag.StartY   = Io.MousePos.y;
  m_Drag.LaneLeft = _Min.x;

  if (KeyId == InvalidTimelineId)
  {
    m_Drag.Mode     = ETimelineDragMode::BoxSelect;
    m_Drag.IsAdding = Io.KeyCtrl || Io.KeyShift;

    if (!m_Drag.IsAdding)
      _Presenter.SelectTrack(_Track.Id);

    return;
  }

  if (Io.KeyCtrl)
  {
    _Presenter.SelectItem(KeyId, ETimelineSelectMode::Toggle);
    return;
  }

  if (!_Presenter.GetSelection().IsSelected(KeyId))
    _Presenter.SelectItem(KeyId, Io.KeyShift ? ETimelineSelectMode::Add : ETimelineSelectMode::Replace);

  m_Drag.Mode = ETimelineDragMode::MoveKeys;
}

void CTimelineView::HandleLaneDrag(ITimelinePresenter & _Presenter, int _FrameRate)
{
  if (m_Drag.Mode != ETimelineDragMode::MoveKeys)
    return;

  const auto & Io    = ImGui::GetIO();
  auto         Delta = m_Range.ToTime(Io.MousePos.x - m_Drag.LaneLeft) - m_Range.ToTime(m_Drag.StartX - m_Drag.LaneLeft);

  if (m_IsSnapping && !Io.KeyAlt)
    Delta = SnapDeltaToFrame(Delta, _FrameRate);

  if (Delta == m_Drag.Delta)
    return;

  m_Drag.Delta   = Delta;
  m_Drag.IsMoved = true;

  _Presenter.PreviewMoveSelection(Delta);
}

void CTimelineView::HandleLaneRelease(ITimelinePresenter & _Presenter)
{
  if (m_Drag.Mode == ETimelineDragMode::BoxSelect)
  {
    m_Drag.IsPending = true;
    return;
  }

  if (m_Drag.Mode == ETimelineDragMode::MoveKeys && m_Drag.IsMoved)
    _Presenter.CommitMove();

  m_Drag = TTimelineDrag();
}

TTimelineTicks CTimelineView::GetMouseTime(const ImVec2 & _Min, int _FrameRate) const
{
  const auto Time = m_Range.ToTime(ImGui::GetIO().MousePos.x - _Min.x);
  return std::max<TTimelineTicks>(m_IsSnapping ? SnapToFrame(Time, _FrameRate) : Time, 0);
}

void CTimelineView::ScrubTo(ITimelinePresenter & _Presenter, float _Offset, int _FrameRate)
{
  m_CurrentTime = std::max<TTimelineTicks>(SnapToFrame(m_Range.ToTime(_Offset), _FrameRate), 0);
  _Presenter.Scrub(m_CurrentTime);
}

} // namespace editor
