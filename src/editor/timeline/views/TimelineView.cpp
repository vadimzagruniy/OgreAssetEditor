#include "TimelineView.h"
#include "TimelineTime.h"
#include "TimelineTimeFormat.h"
#include "editor/timeline/model/Timeline.h"
#include "editor/timeline/presenter/ITimelinePresenter.h"
#include "editor/timeline/presenter/TimelineSelectMode.h"
#include "editor/timeline/presenter/TimelineSelection.h"
#include "editor/timeline/tracks/TimelineItemKind.h"

#include <imgui.h>
#include <imgui_internal.h>

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
constexpr float KeyRadius        = 6.0f;
constexpr float KeyHitRadius     = 8.0f;
constexpr float KeySpanWidth     = 2.0f;
constexpr float KeySpanAlpha     = 0.5f;
constexpr float SummaryKeyScale  = 0.6f;
constexpr float ClipInset        = 2.0f;
constexpr float ZoomStep         = 1.15f;
constexpr float WheelScrollStep  = 40.0f;
constexpr float PlayheadWidth    = 2.0f;
constexpr float PlayheadPadding  = 4.0f;
constexpr float FormatComboScale = 7.0f;
constexpr float SpeedComboScale  = 4.5f;
constexpr float SelectedLaneTint = 0.4f;
constexpr float FrameRounding    = 4.0f;
constexpr float MinBoxSize       = 3.0f;
constexpr ImU32 OutOfRangeShade  = IM_COL32(0, 0, 0, 70);
constexpr ImU32 MissingTarget    = IM_COL32(230, 150, 60, 255);
constexpr ImU32 PlayheadColor    = IM_COL32(235, 75, 60, 255);
constexpr ImU32 PlayheadText     = IM_COL32(255, 255, 255, 255);
constexpr ImU32 KeyBorderColor   = IM_COL32(0, 0, 0, 170);
constexpr ImU32 SelectedKeyRing  = IM_COL32(150, 235, 70, 255);
constexpr float SelectedRingGap  = 1.5f;
constexpr float SelectedRingSize = 3.0f;
constexpr float ZoomSliderScale  = 10.0f;
constexpr float DurationWidth    = 4.5f;
constexpr float DurationGrab     = 5.0f;
constexpr float DurationHandle   = 3.0f;

constexpr std::array<const char *, 2> TimeFormatNames{"Seconds", "Frames"};

constexpr std::array<const char *, 3>           InterpolationNames{"Step", "Linear", "Spline"};
constexpr std::array<ETimelineInterpolation, 3> InterpolationValues{ETimelineInterpolation::Step, ETimelineInterpolation::Linear, ETimelineInterpolation::Spline};
constexpr std::array<const char *, 4> SpeedNames{"x1", "x1/2", "x1/4", "x1/8"};
constexpr std::array<float, 4>        SpeedValues{1.0f, 0.5f, 0.25f, 0.125f};

float GetRowHeight()
{
  return ImGui::GetFrameHeight();
}

float GetCenterY(const ImVec2 & _Min, const ImVec2 & _Max)
{
  return (_Min.y + _Max.y) * 0.5f;
}

std::string GetTrackLabel(const ITimelinePresenter & _Presenter, const TTimelineTrack & _Track)
{
  const auto Label = _Presenter.GetTrackLabel(_Track);
  return Label.empty() ? _Track.TypeId : Label;
}

bool IsTargetRow(const TTimelineTrack & _Track)
{
  return _Track.TypeId == TimelineGroupType && !_Track.Target.Slot.empty();
}

std::string GetClipLabel(const TTimelineClip & _Clip)
{
  const auto SlashPosition = _Clip.Source.find_last_of("/\\");
  return SlashPosition == std::string::npos ? _Clip.Source : _Clip.Source.substr(SlashPosition + 1);
}

float CalcButtonWidth(const char * _Label)
{
  return ImGui::CalcTextSize(_Label).x + ImGui::GetStyle().FramePadding.x * 2.0f;
}

float CalcCheckboxWidth(const char * _Label)
{
  return ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize(_Label).x;
}

float CalcSmallButtonWidth(const char * _Label)
{
  return ImGui::CalcTextSize(_Label).x + ImGui::GetStyle().FramePadding.x * 2.0f;
}

std::string TrimToWidth(const std::string & _Text, float _Width)
{
  if (ImGui::CalcTextSize(_Text.c_str()).x <= _Width)
    return _Text;

  const float Ellipsis = ImGui::CalcTextSize("...").x;
  std::string Result   = _Text;

  while (!Result.empty() && ImGui::CalcTextSize(Result.c_str()).x + Ellipsis > _Width)
  {
    Result.pop_back();

    while (!Result.empty() && (static_cast<unsigned char>(Result.back()) & 0xC0) == 0x80)
      Result.pop_back();
  }

  return Result + "...";
}

float CalcSeparatorWidth()
{
  return ImGui::GetStyle().ItemSpacing.x * 2.0f + 1.0f;
}

void DrawToolbarSeparator()
{
  ImGui::SameLine();
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
  ImGui::SameLine();
}

ImU32 ScaleAlpha(ImU32 _Color, float _Alpha)
{
  ImVec4 Color = ImGui::ColorConvertU32ToFloat4(_Color);
  Color.w *= _Alpha;

  return ImGui::ColorConvertFloat4ToU32(Color);
}

TTimelineTicks FloorToStep(TTimelineTicks _Time, TTimelineTicks _Step)
{
  return _Time / _Step * _Step;
}

TTimelineTicks FramesToTicks(int _Frames, int _FrameRate)
{
  return static_cast<TTimelineTicks>(std::llround(static_cast<double>(_Frames) * static_cast<double>(TimelineTicksPerSecond) / static_cast<double>(std::max(_FrameRate, 1))));
}

int TicksToFrames(TTimelineTicks _Time, int _FrameRate)
{
  return static_cast<int>(std::llround(static_cast<double>(_Time) * static_cast<double>(std::max(_FrameRate, 1)) / static_cast<double>(TimelineTicksPerSecond)));
}

} // namespace

//
// Constructor
//

CTimelineView::CTimelineView():
  m_Format(ETimelineTimeFormat::Seconds),
  m_CurrentTime(0),
  m_RenamingTrack(InvalidTimelineId),
  m_ContextTime(0),
  m_ContextTrack(InvalidTimelineId),
  m_ContextInterpolation(ETimelineInterpolation::Linear),
  m_DraggedDuration(0),
  m_RenameBuffer{},
  m_LanesLeft(0.0f),
  m_LanesRight(0.0f),
  m_LanesTop(0.0f),
  m_IsRenameFocusPending(false),
  m_IsSnapping(true),
  m_IsFitPending(true),
  m_IsDurationHovered(false),
  m_IsDraggingDuration(false)
{
  // Empty
}

//
// Interface
//

void CTimelineView::Draw(ITimelinePresenter & _Presenter, const std::function<void(float)> & _DrawDocumentBar)
{
  const auto & Timeline = _Presenter.GetTimeline();

  m_CurrentTime = _Presenter.GetTime();
  m_KeyHits.clear();

  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, FrameRounding);
  const bool IsVisible = ImGui::BeginChild("##TimelineFrame", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
  ImGui::PopStyleVar();

  if (IsVisible)
  {
    DrawToolbar(Timeline, _Presenter, _DrawDocumentBar);
    DrawTracks(Timeline, _Presenter);
  }

  ImGui::EndChild();
}

//
// Implementation
//

void CTimelineView::DrawTracks(const CTimeline & _Timeline, ITimelinePresenter & _Presenter)
{
  const ImGuiTableFlags TableFlags   = ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg;
  const float           BottomHeight = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 3.0f + 1.0f;

  if (!ImGui::BeginTable("##TimelineTracks", 2, TableFlags, ImVec2(0.0f, -BottomHeight)))
    return;

  ImGui::TableSetupScrollFreeze(0, 1);
  ImGui::TableSetupColumn("Tracks", ImGuiTableColumnFlags_WidthFixed, TrackColumnWidth);
  ImGui::TableSetupColumn("Lanes", ImGuiTableColumnFlags_WidthStretch);

  DrawRulerRow(_Timeline, _Presenter);

  for (const auto & Track : _Timeline.GetTracks())
    DrawTrackRow(_Presenter, Track);

  if (_Timeline.GetTracks().empty())
  {
    ImGui::TableNextRow(ImGuiTableRowFlags_None, GetRowHeight());
    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("No tracks");
  }

  DrawDurationMarker(_Timeline);
  DrawPlayhead(_Timeline);
  DrawSelectionBox(_Presenter);

  ImGui::EndTable();

  DrawZoomBar();

  if (ImGui::Shortcut(ImGuiKey_Delete))
    _Presenter.DeleteSelection();

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_C))
    _Presenter.CopySelection();

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_V) && _Presenter.CanPasteKeys())
    _Presenter.PasteKeys(GetPlayheadKeyTime(_Presenter));

  if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D))
    _Presenter.DuplicateSelection();
}

void CTimelineView::DrawToolbar(const CTimeline & _Timeline, ITimelinePresenter & _Presenter, const std::function<void(float)> & _DrawDocumentBar)
{
  DrawPlaybackButtons(_Presenter);
  DrawTimeText(_Timeline);
  DrawDurationField(_Timeline, _Presenter);

  if (_DrawDocumentBar)
  {
    DrawToolbarSeparator();
    _DrawDocumentBar(std::max(ImGui::GetContentRegionAvail().x - CalcSeparatorWidth() - CalcViewOptionsWidth(), 0.0f));
  }

  DrawViewOptions(_Presenter);
}

void CTimelineView::DrawPlaybackButtons(ITimelinePresenter & _Presenter) const
{
  const bool  IsPlaying = _Presenter.IsPlaying();
  const float PlayWidth = std::max(CalcButtonWidth("Play"), CalcButtonWidth("Pause"));

  if (ImGui::Button(IsPlaying ? "Pause###Play" : "Play###Play", ImVec2(PlayWidth, 0.0f)))
  {
    if (IsPlaying)
      _Presenter.Pause();
    else
      _Presenter.Play();
  }

  ImGui::SetItemTooltip(IsPlaying ? "Pause the preview" : "Play the animation in the scene");

  ImGui::SameLine();
  ImGui::BeginDisabled(!_Presenter.IsPreviewing());
  if (ImGui::Button("Stop"))
    _Presenter.Stop();
  ImGui::EndDisabled();

  ImGui::SetItemTooltip("Stop the preview and restore the scene");
}

void CTimelineView::DrawTimeText(const CTimeline & _Timeline) const
{
  const auto  FrameRate    = _Timeline.GetFrameRate();
  const auto  Duration     = _Timeline.GetDuration();
  const auto  LongestTime  = std::max<TTimelineTicks>(Duration - TimelineTicksPerSecond / 1000, 0);
  const auto  CurrentText  = FormatTimelineTime(m_CurrentTime, m_Format, FrameRate);
  const auto  DurationText = FormatTimelineTime(Duration, m_Format, FrameRate);
  const auto  LongestText  = FormatTimelineTime(LongestTime, m_Format, FrameRate);
  const float CurrentWidth = ImGui::CalcTextSize(CurrentText.c_str()).x;
  const float SlotWidth    = std::max({CurrentWidth, ImGui::CalcTextSize(DurationText.c_str()).x, ImGui::CalcTextSize(LongestText.c_str()).x});

  ImGui::SameLine();
  ImGui::AlignTextToFramePadding();

  const float SlotLeft = ImGui::GetCursorPosX();

  ImGui::SetCursorPosX(SlotLeft + SlotWidth - CurrentWidth);
  ImGui::TextUnformatted(CurrentText.c_str());
  ImGui::SetItemTooltip("Playhead time");

}

void CTimelineView::DrawDurationField(const CTimeline & _Timeline, ITimelinePresenter & _Presenter) const
{
  const int            FrameRate   = _Timeline.GetFrameRate();
  const TTimelineTicks MinDuration = GetFrameTicks(FrameRate);

  ImGui::SameLine(0.0f, 0.0f);
  ImGui::TextDisabled(" / ");

  ImGui::SameLine(0.0f, 0.0f);
  ImGui::SetNextItemWidth(ImGui::GetFontSize() * DurationWidth);

  if (m_Format == ETimelineTimeFormat::Frames)
  {
    int Frames = TicksToFrames(GetShownDuration(_Timeline), FrameRate);
    ImGui::InputInt("##Duration", &Frames, 0, 0);

    if (ImGui::IsItemDeactivatedAfterEdit())
      _Presenter.SetDuration(std::max(FramesToTicks(Frames, FrameRate), MinDuration));
  }
  else
  {
    double Seconds = static_cast<double>(GetShownDuration(_Timeline)) / static_cast<double>(TimelineTicksPerSecond);
    ImGui::InputDouble("##Duration", &Seconds, 0.0, 0.0, "%.3f");

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
      const auto Duration = static_cast<TTimelineTicks>(std::llround(Seconds * static_cast<double>(TimelineTicksPerSecond)));
      _Presenter.SetDuration(std::max(m_IsSnapping ? SnapToFrame(Duration, FrameRate) : Duration, MinDuration));
    }
  }

  ImGui::SetItemTooltip("Animation length, also drag its end on the ruler");
}

float CTimelineView::CalcViewOptionsWidth() const
{
  const float Spacing = ImGui::GetStyle().ItemSpacing.x;

  return ImGui::GetFontSize() * (SpeedComboScale + FormatComboScale) + CalcCheckboxWidth("Loop") + CalcCheckboxWidth("Snap") + Spacing * 3.0f;
}

void CTimelineView::DrawViewOptions(ITimelinePresenter & _Presenter)
{
  const float RightWidth = CalcSeparatorWidth() - ImGui::GetStyle().ItemSpacing.x + CalcViewOptionsWidth();

  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(ImGui::GetContentRegionAvail().x - RightWidth, 0.0f));
  ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

  ImGui::SameLine();
  DrawSpeedCombo(_Presenter);

  int FormatIndex = static_cast<int>(m_Format);

  ImGui::SameLine();
  ImGui::SetNextItemWidth(ImGui::GetFontSize() * FormatComboScale);
  if (ImGui::Combo("##TimeFormat", &FormatIndex, TimeFormatNames.data(), static_cast<int>(TimeFormatNames.size())))
    m_Format = static_cast<ETimelineTimeFormat>(FormatIndex);

  ImGui::SetItemTooltip("Show time in seconds or frames");

  bool IsLooping = _Presenter.IsLooping();

  ImGui::SameLine();
  if (ImGui::Checkbox("Loop", &IsLooping))
    _Presenter.SetLooping(IsLooping);

  ImGui::SetItemTooltip("Repeat the preview from the start");

  ImGui::SameLine();
  ImGui::Checkbox("Snap", &m_IsSnapping);
  ImGui::SetItemTooltip("Snap keys to frames, hold Alt to move freely");
}

void CTimelineView::DrawSpeedCombo(ITimelinePresenter & _Presenter) const
{
  const float Speed        = _Presenter.GetPlaybackSpeed();
  const auto  CurrentSpeed = std::find(SpeedValues.begin(), SpeedValues.end(), Speed);
  const auto  CurrentIndex = CurrentSpeed == SpeedValues.end() ? 0 : std::distance(SpeedValues.begin(), CurrentSpeed);

  ImGui::SetNextItemWidth(ImGui::GetFontSize() * SpeedComboScale);

  if (ImGui::BeginCombo("##Speed", SpeedNames[CurrentIndex]))
  {
    for (std::size_t SpeedIndex = 0; SpeedIndex < SpeedValues.size(); ++SpeedIndex)
    {
      if (ImGui::Selectable(SpeedNames[SpeedIndex], static_cast<std::ptrdiff_t>(SpeedIndex) == CurrentIndex))
        _Presenter.SetPlaybackSpeed(SpeedValues[SpeedIndex]);
    }

    ImGui::EndCombo();
  }

  ImGui::SetItemTooltip("Playback speed of the preview");
}

void CTimelineView::DrawAddTrackMenu(ITimelinePresenter & _Presenter) const
{
  const bool IsClicked = ImGui::Button("+ Add Track", ImVec2(-1.0f, 0.0f));
  ImGui::SetItemTooltip("Add a track for the selected scene node");

  if (IsClicked && !_Presenter.HasTrackTarget())
  {
    _Presenter.RequestTrackTarget();
  }
  else
  if (IsClicked || _Presenter.IsAddTrackMenuRequested())
  {
    ImGui::OpenPopup("##AddTrackMenu");
    _Presenter.ClearAddTrackMenuRequest();
  }

  ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y), ImGuiCond_Appearing);

  if (!ImGui::BeginPopup("##AddTrackMenu"))
    return;

  const auto Templates = _Presenter.GetTrackTemplates();

  for (const auto & Template : Templates)
  {
    if (ImGui::MenuItem(Template.Label.c_str()))
      _Presenter.AddTrack(Template.Track);
  }

  if (Templates.empty())
    ImGui::TextDisabled(_Presenter.HasTrackTarget() ? "The node already has all its tracks" : "No scene node selected");

  ImGui::Separator();

  if (ImGui::MenuItem("Choose Scene Node..."))
    _Presenter.RequestTrackTarget();

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
  ImGui::TableSetColumnIndex(0);
  DrawAddTrackMenu(_Presenter);

  ImGui::TableSetColumnIndex(1);

  const ImVec2 Min   = ImGui::GetCursorScreenPos();
  const float  Width = std::max(ImGui::GetContentRegionAvail().x, 1.0f);
  const ImVec2 Max(Min.x + Width, Min.y + RowHeight);

  m_LanesLeft  = Min.x;
  m_LanesRight = Max.x;
  m_LanesTop   = Min.y;

  if (m_IsFitPending)
  {
    m_Range.Fit(_Timeline.GetDuration(), Width);
    m_IsFitPending = false;
  }

  ImGui::InvisibleButton("##Ruler", ImVec2(Width, RowHeight));
  HandleRulerMouse(_Timeline, _Presenter, Min);
  ImGui::SetItemTooltip(m_IsDurationHovered ? "Drag to change the animation length" : "Click or drag to move the playhead");

  HandleNavigation(Min);

  auto & DrawList = *ImGui::GetWindowDrawList();
  DrawList.AddRectFilled(Min, Max, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
  DrawRulerTicks(DrawList, Min, Max, _Timeline);
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

  const float DurationX = _Min.x + m_Range.ToOffset(GetShownDuration(_Timeline));
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

  const bool  IsRenaming   = m_RenamingTrack == _Track.Id;
  const bool  HasAddKey    = !IsRenaming && _Presenter.GetItemKind(_Track) == ETimelineItemKind::Keys;
  const float CellLeft     = ImGui::GetCursorPosX();
  const float CellRight    = CellLeft + ImGui::GetContentRegionAvail().x;
  const float ButtonSpace  = HasAddKey ? CalcSmallButtonWidth("+") + ImGui::GetStyle().ItemSpacing.x : 0.0f;
  const float LabelWidth   = CellRight - CellLeft - ImGui::GetTreeNodeToLabelSpacing() - ButtonSpace;
  const auto  FullLabel    = GetTrackLabel(_Presenter, _Track);
  const auto  ShownLabel   = IsRenaming ? std::string() : TrimToWidth(FullLabel, LabelWidth);
  const bool  IsMissing    = !_Presenter.IsTargetValid(_Track);

  if (IsMissing)
    ImGui::PushStyleColor(ImGuiCol_Text, MissingTarget);

  ImGui::AlignTextToFramePadding();
  const bool IsOpen = ImGui::TreeNodeEx("##Track", Flags, "%s", ShownLabel.c_str());

  if (IsMissing)
  {
    ImGui::PopStyleColor();
    ImGui::SetItemTooltip("%s\nTarget node '%s' is not in the scene", FullLabel.c_str(), _Track.Target.Slot.c_str());
  }
  else
  if (ShownLabel != FullLabel && !IsRenaming)
    ImGui::SetItemTooltip("%s", FullLabel.c_str());

  if ((ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemToggledOpen())
    _Presenter.SelectTrack(_Track.Id);

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen() && !_Track.Target.Slot.empty())
    _Presenter.ShowTrackInScene(_Track.Id);

  DrawTrackContextMenu(_Presenter, _Track);

  if (IsRenaming)
  {
    ImGui::SameLine();
    DrawRenameField(_Presenter);
    return IsOpen;
  }

  if (HasAddKey)
    DrawAddKeyButton(_Presenter, _Track, CellRight);

  return IsOpen;
}

void CTimelineView::DrawTrackContextMenu(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track)
{
  if (!ImGui::BeginPopupContextItem("##TrackMenu"))
    return;

  if (IsTargetRow(_Track))
    DrawTargetMenu(_Presenter, _Track);

  if (ImGui::MenuItem("Rename", nullptr, false, !IsTargetRow(_Track)))
    BeginRename(_Track);

  if (ImGui::MenuItem("Bind to Selected Node", nullptr, false, _Presenter.CanBindToSelection(_Track)))
    _Presenter.BindToSelection(_Track);

  ImGui::Separator();

  const bool HasKeys = _Presenter.GetItemKind(_Track) == ETimelineItemKind::Keys;

  if (ImGui::MenuItem("Insert Key at Playhead", nullptr, false, HasKeys && _Presenter.CanAddKey(_Track.Id)))
    _Presenter.AddKey(_Track.Id, GetPlayheadKeyTime(_Presenter));

  if (ImGui::MenuItem("Select All Keys", nullptr, false, !_Track.Keys.empty()))
  {
    std::vector<TTimelineId> KeyIds;

    for (const auto & Key : _Track.Keys)
      KeyIds.push_back(Key.Id);

    _Presenter.SelectItems(KeyIds, false);
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Delete", "Del"))
    _Presenter.DeleteTrack(_Track.Id);

  ImGui::EndPopup();
}

void CTimelineView::DrawTargetMenu(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track) const
{
  if (ImGui::BeginMenu("Add Track"))
  {
    const auto Templates = _Presenter.GetTargetTemplates(_Track);

    for (const auto & Template : Templates)
    {
      if (ImGui::MenuItem(Template.Label.c_str()))
        _Presenter.AddTrack(Template.Track);
    }

    if (Templates.empty())
      ImGui::TextDisabled("The node already has all its tracks");

    ImGui::EndMenu();
  }

  if (ImGui::MenuItem("Select in Scene", nullptr, false, _Presenter.IsTargetValid(_Track)))
    _Presenter.ShowTrackInScene(_Track.Id);

  ImGui::Separator();
}

void CTimelineView::DrawAddKeyButton(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track, float _CellRight) const
{
  ImGui::SameLine();
  ImGui::SetCursorPosX(_CellRight - CalcSmallButtonWidth("+"));

  ImGui::BeginDisabled(!_Presenter.CanAddKey(_Track.Id));
  if (ImGui::SmallButton("+"))
    _Presenter.AddKey(_Track.Id, GetPlayheadKeyTime(_Presenter));
  ImGui::EndDisabled();

  ImGui::SetItemTooltip("Add a key at the playhead");
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
  const auto Label  = _Track.Name.empty() ? _Track.TypeId : _Track.Name;
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
  HandleLaneContextClick(_Presenter, _Track, Min, Max);
  CollectKeyHits(Min, Max, _Track);
  DrawKeyContextMenu(_Presenter);
  DrawLaneContextMenu(_Presenter);

  auto & DrawList = *ImGui::GetWindowDrawList();

  if (Selection.GetTrack() == _Track.Id)
    DrawList.AddRectFilled(Min, Max, ImGui::GetColorU32(ImGuiCol_Header, SelectedLaneTint));

  DrawGrid(DrawList, Min, Max, Timeline);
  DrawClips(DrawList, Min, Max, _Track);
  const ImU32 TrackColor = _Presenter.GetTrackColor(_Track);

  DrawKeySpan(DrawList, Min, Max, _Track, TrackColor);
  DrawKeys(DrawList, Min, Max, _Track, Selection, TrackColor);

  for (const auto & Child : _Track.Children)
    DrawSummaryKeys(DrawList, Min, Max, Child);
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

  const float DurationX = _Min.x + m_Range.ToOffset(GetShownDuration(_Timeline));
  if (DurationX < _Max.x)
    _DrawList.AddRectFilled(ImVec2(std::max(DurationX, _Min.x), _Min.y), _Max, OutOfRangeShade);
}

void CTimelineView::DrawKeySpan(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, unsigned int _Color) const
{
  if (_Track.Keys.size() < 2)
    return;

  const auto IsEarlier           = [](const TTimelineKey & _Left, const TTimelineKey & _Right) { return _Left.Time < _Right.Time; };
  const auto [FirstKey, LastKey] = std::minmax_element(_Track.Keys.begin(), _Track.Keys.end(), IsEarlier);

  const float CenterY = GetCenterY(_Min, _Max);
  const float StartX  = std::max(_Min.x + m_Range.ToOffset(FirstKey->Time), _Min.x);
  const float EndX    = std::min(_Min.x + m_Range.ToOffset(LastKey->Time), _Max.x);

  if (StartX >= EndX)
    return;

  _DrawList.AddLine(ImVec2(StartX, CenterY), ImVec2(EndX, CenterY), ScaleAlpha(_Color, KeySpanAlpha), KeySpanWidth);
}

void CTimelineView::DrawKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track, const CTimelineSelection & _Selection, unsigned int _Color) const
{
  const float  CenterY     = GetCenterY(_Min, _Max);
  const ImVec2 Mouse       = ImGui::GetIO().MousePos;
  const bool   IsHoverable = ImGui::IsWindowHovered() && m_Drag.Mode == ETimelineDragMode::None;

  for (const auto & Key : _Track.Keys)
  {
    const float PositionX = _Min.x + m_Range.ToOffset(Key.Time);
    if (PositionX < _Min.x - KeyRadius || PositionX > _Max.x + KeyRadius)
      continue;

    const ImVec2 Center(PositionX, CenterY);
    const bool   IsSelected = _Selection.IsSelected(Key.Id);
    const bool   IsHovered  = IsHoverable && std::abs(Mouse.x - PositionX) <= KeyHitRadius && std::abs(Mouse.y - CenterY) <= KeyHitRadius;
    const float  Radius     = IsSelected || IsHovered ? KeyRadius + 1.0f : KeyRadius;

    if (IsSelected)
      _DrawList.AddCircleFilled(Center, Radius + SelectedRingGap, SelectedKeyRing);

    _DrawList.AddCircleFilled(Center, Radius, _Color);
  }
}

void CTimelineView::DrawSummaryKeys(ImDrawList & _DrawList, const ImVec2 & _Min, const ImVec2 & _Max, const TTimelineTrack & _Track) const
{
  const float CenterY = GetCenterY(_Min, _Max);
  const float Radius  = KeyRadius * SummaryKeyScale;
  const ImU32 Color   = ImGui::GetColorU32(ImGuiCol_TextDisabled);

  for (const auto & Key : _Track.Keys)
  {
    const float PositionX = _Min.x + m_Range.ToOffset(Key.Time);
    if (PositionX >= _Min.x - Radius && PositionX <= _Max.x + Radius)
      _DrawList.AddCircleFilled(ImVec2(PositionX, CenterY), Radius, Color);
  }

  for (const auto & Child : _Track.Children)
    DrawSummaryKeys(_DrawList, _Min, _Max, Child);
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

void CTimelineView::DrawDurationMarker(const CTimeline & _Timeline) const
{
  const float PositionX = m_LanesLeft + m_Range.ToOffset(GetShownDuration(_Timeline));
  if (PositionX < m_LanesLeft || PositionX > m_LanesRight)
    return;

  const bool   IsActive  = m_IsDurationHovered || m_IsDraggingDuration;
  const ImU32  Color     = ImGui::GetColorU32(IsActive ? ImGuiCol_Text : ImGuiCol_TextDisabled);
  const float  Bottom    = ImGui::GetWindowPos().y + ImGui::GetWindowHeight();
  const ImVec2 HandleMin(PositionX - DurationHandle, m_LanesTop + 1.0f);
  const ImVec2 HandleMax(PositionX + DurationHandle, m_LanesTop + GetRowHeight() - 1.0f);
  auto &       DrawList  = *ImGui::GetWindowDrawList();

  DrawList.PushClipRect(ImVec2(m_LanesLeft, m_LanesTop), ImVec2(m_LanesRight, Bottom), false);
  DrawList.AddLine(ImVec2(PositionX, m_LanesTop), ImVec2(PositionX, Bottom), Color);
  DrawList.AddRectFilled(HandleMin, HandleMax, Color, 1.0f);
  DrawList.PopClipRect();
}

void CTimelineView::DrawPlayhead(const CTimeline & _Timeline) const
{
  const float PositionX = m_LanesLeft + m_Range.ToOffset(m_CurrentTime);
  if (PositionX < m_LanesLeft || PositionX > m_LanesRight)
    return;

  const float  RowHeight = GetRowHeight();
  const float  Bottom    = ImGui::GetWindowPos().y + ImGui::GetWindowHeight();
  const auto   Label     = FormatTimelineTime(m_CurrentTime, m_Format, _Timeline.GetFrameRate());
  const ImVec2 LabelSize = ImGui::CalcTextSize(Label.c_str());
  const float  HalfWidth = LabelSize.x * 0.5f + PlayheadPadding;
  const float  TagTop    = m_LanesTop + 1.0f;
  const float  TagBottom = m_LanesTop + RowHeight - PlayheadPadding;
  const ImVec2 LabelPosition(PositionX - LabelSize.x * 0.5f, TagTop + (TagBottom - TagTop - LabelSize.y) * 0.5f);
  auto &       DrawList  = *ImGui::GetWindowDrawList();

  DrawList.PushClipRect(ImVec2(m_LanesLeft, m_LanesTop), ImVec2(m_LanesRight, Bottom), false);

  DrawList.AddLine(ImVec2(PositionX, TagBottom), ImVec2(PositionX, Bottom), PlayheadColor, PlayheadWidth);
  DrawList.AddRectFilled(ImVec2(PositionX - HalfWidth, TagTop), ImVec2(PositionX + HalfWidth, TagBottom), PlayheadColor, 3.0f);
  DrawList.AddTriangleFilled(ImVec2(PositionX - PlayheadPadding, TagBottom), ImVec2(PositionX + PlayheadPadding, TagBottom), ImVec2(PositionX, m_LanesTop + RowHeight), PlayheadColor);
  DrawList.AddText(LabelPosition, PlayheadText, Label.c_str());

  DrawList.PopClipRect();
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

void CTimelineView::DrawKeyContextMenu(ITimelinePresenter & _Presenter)
{
  if (!ImGui::BeginPopup("##KeyMenu"))
    return;

  if (ImGui::MenuItem("Go to Key"))
    _Presenter.Scrub(m_ContextTime);

  ImGui::Separator();

  if (ImGui::MenuItem("Copy", "Ctrl+C"))
    _Presenter.CopySelection();

  if (ImGui::MenuItem("Paste at Playhead", "Ctrl+V", false, _Presenter.CanPasteKeys()))
    _Presenter.PasteKeys(GetPlayheadKeyTime(_Presenter));

  if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
    _Presenter.DuplicateSelection();

  ImGui::Separator();

  if (ImGui::BeginMenu("Interpolation"))
  {
    for (std::size_t InterpolationIndex = 0; InterpolationIndex < InterpolationValues.size(); ++InterpolationIndex)
    {
      const auto Interpolation = InterpolationValues[InterpolationIndex];

      if (ImGui::MenuItem(InterpolationNames[InterpolationIndex], nullptr, Interpolation == m_ContextInterpolation))
        _Presenter.SetSelectionInterpolation(Interpolation);
    }

    ImGui::EndMenu();
  }

  ImGui::Separator();

  if (ImGui::MenuItem("Delete", "Del"))
    _Presenter.DeleteSelection();

  ImGui::EndPopup();
}

void CTimelineView::DrawLaneContextMenu(ITimelinePresenter & _Presenter)
{
  if (!ImGui::BeginPopup("##LaneMenu"))
    return;

  if (ImGui::MenuItem("Add Key Here", nullptr, false, _Presenter.CanAddKey(m_ContextTrack)))
    _Presenter.AddKey(m_ContextTrack, m_ContextTime);

  if (ImGui::MenuItem("Paste Here", nullptr, false, _Presenter.CanPasteKeys()))
    _Presenter.PasteKeys(m_ContextTime);

  ImGui::EndPopup();
}

void CTimelineView::DrawZoomBar()
{
  const float SliderWidth  = ImGui::GetFontSize() * ZoomSliderScale;
  const float BarWidth     = CalcButtonWidth("Fit") + ImGui::GetStyle().ItemSpacing.x + SliderWidth;
  const float AnchorOffset = std::clamp(m_Range.ToOffset(m_CurrentTime), 0.0f, m_LanesRight - m_LanesLeft);

  ImGui::Separator();

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(ImGui::GetContentRegionAvail().x - BarWidth, 0.0f));

  if (ImGui::Button("Fit"))
    m_IsFitPending = true;

  ImGui::SetItemTooltip("Fit the whole animation into the view");

  ImGui::SameLine();
  ImGui::SetNextItemWidth(SliderWidth);

  float ZoomLevel = m_Range.GetZoomLevel();
  if (ImGui::SliderFloat("##Zoom", &ZoomLevel, 0.0f, 1.0f, "Zoom"))
    m_Range.SetZoomLevel(ZoomLevel, AnchorOffset);

  ImGui::SetItemTooltip("Timeline zoom, also Ctrl + mouse wheel over the tracks");
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

void CTimelineView::HandleRulerMouse(const CTimeline & _Timeline, ITimelinePresenter & _Presenter, const ImVec2 & _Min)
{
  const auto &  Io        = ImGui::GetIO();
  const int     FrameRate = _Timeline.GetFrameRate();
  const float   DurationX = _Min.x + m_Range.ToOffset(GetShownDuration(_Timeline));

  m_IsDurationHovered = ImGui::IsItemHovered() && std::abs(Io.MousePos.x - DurationX) <= DurationGrab;

  if (m_IsDurationHovered || m_IsDraggingDuration)
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

  if (ImGui::IsItemActivated() && m_IsDurationHovered)
  {
    m_IsDraggingDuration = true;
    m_DraggedDuration    = _Timeline.GetDuration();
  }

  if (ImGui::IsItemActive())
  {
    if (m_IsDraggingDuration)
      m_DraggedDuration = std::max(GetMouseTime(_Min, FrameRate), GetFrameTicks(FrameRate));
    else
      ScrubTo(_Presenter, Io.MousePos.x - _Min.x, FrameRate);
  }

  if (!ImGui::IsItemDeactivated() || !m_IsDraggingDuration)
    return;

  m_IsDraggingDuration = false;
  _Presenter.SetDuration(m_DraggedDuration);
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

  for (const auto & Key : _Track.Keys)
  {
    if (Key.Id == KeyId)
      m_Drag.Anchor = Key.Time;
  }

  m_Drag.Mode = ETimelineDragMode::MoveKeys;
}

void CTimelineView::HandleLaneDrag(ITimelinePresenter & _Presenter, int _FrameRate)
{
  if (m_Drag.Mode != ETimelineDragMode::MoveKeys)
    return;

  const auto & Io    = ImGui::GetIO();
  auto         Delta = m_Range.ToTime(Io.MousePos.x - m_Drag.LaneLeft) - m_Range.ToTime(m_Drag.StartX - m_Drag.LaneLeft);

  if (m_IsSnapping && !Io.KeyAlt)
    Delta = SnapTime(m_Drag.Anchor + Delta, _FrameRate) - m_Drag.Anchor;

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

void CTimelineView::HandleLaneContextClick(ITimelinePresenter & _Presenter, const TTimelineTrack & _Track, const ImVec2 & _Min, const ImVec2 & _Max)
{
  if (!ImGui::IsItemHovered() || !ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    return;

  const TTimelineId KeyId = FindKeyAt(_Min, _Max, _Track, ImGui::GetIO().MousePos);

  m_ContextTrack = _Track.Id;

  if (KeyId == InvalidTimelineId)
  {
    m_ContextTime = GetMouseTime(_Min, _Presenter.GetTimeline().GetFrameRate());
    ImGui::OpenPopup("##LaneMenu");
    return;
  }

  if (!_Presenter.GetSelection().IsSelected(KeyId))
    _Presenter.SelectItem(KeyId, ETimelineSelectMode::Replace);

  for (const auto & Key : _Track.Keys)
  {
    if (Key.Id != KeyId)
      continue;

    m_ContextTime          = Key.Time;
    m_ContextInterpolation = Key.Interpolation;
  }

  ImGui::OpenPopup("##KeyMenu");
}

TTimelineTicks CTimelineView::GetMouseTime(const ImVec2 & _Min, int _FrameRate) const
{
  const auto Time = m_Range.ToTime(ImGui::GetIO().MousePos.x - _Min.x);
  return std::max<TTimelineTicks>(m_IsSnapping ? SnapTime(Time, _FrameRate) : Time, 0);
}

void CTimelineView::ScrubTo(ITimelinePresenter & _Presenter, float _Offset, int _FrameRate)
{
  const auto Time = m_Range.ToTime(_Offset);

  m_CurrentTime = std::max<TTimelineTicks>(m_IsSnapping ? SnapTime(Time, _FrameRate) : Time, 0);
  _Presenter.Scrub(m_CurrentTime);
}

TTimelineTicks CTimelineView::SnapTime(TTimelineTicks _Time, int _FrameRate) const
{
  const auto GridStep = m_Range.FindGridStep(m_Format, _FrameRate, MinGridSpacing);
  const auto SnapStep = std::max(GridStep / MinorTickCount, GetFrameTicks(_FrameRate));
  const auto GridTime = std::llround(static_cast<double>(_Time) / static_cast<double>(SnapStep)) * SnapStep;

  return SnapToFrame(GridTime, _FrameRate);
}

TTimelineTicks CTimelineView::GetPlayheadKeyTime(const ITimelinePresenter & _Presenter) const
{
  return m_IsSnapping ? SnapToFrame(m_CurrentTime, _Presenter.GetTimeline().GetFrameRate()) : m_CurrentTime;
}

TTimelineTicks CTimelineView::GetShownDuration(const CTimeline & _Timeline) const
{
  return m_IsDraggingDuration ? m_DraggedDuration : _Timeline.GetDuration();
}

} // namespace editor
