#include "EditorStyle.h"

#include <imgui.h>

#include <filesystem>

namespace editor::ui
{

namespace
{

constexpr float FontSize = 15.0f;

// Graphite surfaces from darkest to lightest and a single blue accent.
constexpr unsigned int CanvasColor       = 0x1E1F22;
constexpr unsigned int SunkenColor       = 0x25262A;
constexpr unsigned int PanelColor        = 0x2B2D30;
constexpr unsigned int RaisedColor       = 0x313338;
constexpr unsigned int HoverColor        = 0x35383D;
constexpr unsigned int FrameColor        = 0x393B40;
constexpr unsigned int FrameHoverColor   = 0x43464C;
constexpr unsigned int FrameActiveColor  = 0x4A4D53;
constexpr unsigned int BorderColor       = 0x3C3F44;
constexpr unsigned int GrabColor         = 0x55585E;
constexpr unsigned int TextColor         = 0xDFE1E5;
constexpr unsigned int MutedTextColor    = 0x6F737A;
constexpr unsigned int AccentColor       = 0x3574F0;
constexpr unsigned int AccentHoverColor  = 0x4D86F5;
constexpr unsigned int SelectionColor    = 0x2E436E;
constexpr unsigned int SelectionHotColor = 0x34508A;

ImVec4 ToColor(unsigned int _Rgb, float _Alpha = 1.0f)
{
  const float Red   = static_cast<float>((_Rgb >> 16) & 0xFF) / 255.0f;
  const float Green = static_cast<float>((_Rgb >> 8) & 0xFF) / 255.0f;
  const float Blue  = static_cast<float>(_Rgb & 0xFF) / 255.0f;

  return ImVec4(Red, Green, Blue, _Alpha);
}

void ApplyColors(ImGuiStyle & _Style)
{
  auto & Colors = _Style.Colors;

  Colors[ImGuiCol_Text]                      = ToColor(TextColor);
  Colors[ImGuiCol_TextDisabled]              = ToColor(MutedTextColor);
  Colors[ImGuiCol_WindowBg]                  = ToColor(PanelColor);
  Colors[ImGuiCol_ChildBg]                   = ToColor(PanelColor, 0.0f);
  Colors[ImGuiCol_PopupBg]                   = ToColor(RaisedColor);
  Colors[ImGuiCol_Border]                    = ToColor(BorderColor);
  Colors[ImGuiCol_BorderShadow]              = ToColor(0x000000, 0.0f);

  Colors[ImGuiCol_FrameBg]                   = ToColor(FrameColor);
  Colors[ImGuiCol_FrameBgHovered]            = ToColor(FrameHoverColor);
  Colors[ImGuiCol_FrameBgActive]             = ToColor(FrameActiveColor);

  Colors[ImGuiCol_TitleBg]                   = ToColor(PanelColor);
  Colors[ImGuiCol_TitleBgActive]             = ToColor(PanelColor);
  Colors[ImGuiCol_TitleBgCollapsed]          = ToColor(PanelColor);
  Colors[ImGuiCol_MenuBarBg]                 = ToColor(SunkenColor);

  Colors[ImGuiCol_ScrollbarBg]               = ToColor(PanelColor, 0.0f);
  Colors[ImGuiCol_ScrollbarGrab]             = ToColor(FrameActiveColor);
  Colors[ImGuiCol_ScrollbarGrabHovered]      = ToColor(GrabColor);
  Colors[ImGuiCol_ScrollbarGrabActive]       = ToColor(GrabColor);

  Colors[ImGuiCol_CheckMark]                 = ToColor(AccentColor);
  Colors[ImGuiCol_SliderGrab]                = ToColor(AccentColor);
  Colors[ImGuiCol_SliderGrabActive]          = ToColor(AccentHoverColor);

  Colors[ImGuiCol_Button]                    = ToColor(FrameColor);
  Colors[ImGuiCol_ButtonHovered]             = ToColor(FrameHoverColor);
  Colors[ImGuiCol_ButtonActive]              = ToColor(FrameActiveColor);

  Colors[ImGuiCol_Header]                    = ToColor(SelectionColor);
  Colors[ImGuiCol_HeaderHovered]             = ToColor(HoverColor);
  Colors[ImGuiCol_HeaderActive]              = ToColor(SelectionHotColor);

  Colors[ImGuiCol_Separator]                 = ToColor(BorderColor);
  Colors[ImGuiCol_SeparatorHovered]          = ToColor(AccentColor, 0.6f);
  Colors[ImGuiCol_SeparatorActive]           = ToColor(AccentColor);

  Colors[ImGuiCol_ResizeGrip]                = ToColor(GrabColor, 0.0f);
  Colors[ImGuiCol_ResizeGripHovered]         = ToColor(AccentColor, 0.6f);
  Colors[ImGuiCol_ResizeGripActive]          = ToColor(AccentColor);

  Colors[ImGuiCol_TabHovered]                = ToColor(HoverColor);
  Colors[ImGuiCol_Tab]                       = ToColor(PanelColor);
  Colors[ImGuiCol_TabSelected]               = ToColor(RaisedColor);
  Colors[ImGuiCol_TabSelectedOverline]       = ToColor(AccentColor);
  Colors[ImGuiCol_TabDimmed]                 = ToColor(PanelColor);
  Colors[ImGuiCol_TabDimmedSelected]         = ToColor(RaisedColor);
  Colors[ImGuiCol_TabDimmedSelectedOverline] = ToColor(FrameActiveColor);

  Colors[ImGuiCol_DockingPreview]            = ToColor(AccentColor, 0.5f);
  Colors[ImGuiCol_DockingEmptyBg]            = ToColor(CanvasColor);

  Colors[ImGuiCol_PlotLines]                 = ToColor(AccentColor);
  Colors[ImGuiCol_PlotLinesHovered]          = ToColor(AccentHoverColor);
  Colors[ImGuiCol_PlotHistogram]             = ToColor(AccentColor);
  Colors[ImGuiCol_PlotHistogramHovered]      = ToColor(AccentHoverColor);

  Colors[ImGuiCol_TableHeaderBg]             = ToColor(RaisedColor);
  Colors[ImGuiCol_TableBorderStrong]         = ToColor(BorderColor);
  Colors[ImGuiCol_TableBorderLight]          = ToColor(HoverColor);
  Colors[ImGuiCol_TableRowBg]                = ToColor(0xFFFFFF, 0.0f);
  Colors[ImGuiCol_TableRowBgAlt]             = ToColor(0xFFFFFF, 0.02f);

  Colors[ImGuiCol_TextLink]                  = ToColor(AccentHoverColor);
  Colors[ImGuiCol_TextSelectedBg]            = ToColor(AccentColor, 0.35f);
  Colors[ImGuiCol_DragDropTarget]            = ToColor(AccentColor);
  Colors[ImGuiCol_NavHighlight]              = ToColor(AccentColor);
  Colors[ImGuiCol_NavWindowingHighlight]     = ToColor(TextColor, 0.7f);
  Colors[ImGuiCol_NavWindowingDimBg]         = ToColor(0x000000, 0.4f);
  Colors[ImGuiCol_ModalWindowDimBg]          = ToColor(0x000000, 0.5f);
}

void ApplyMetrics(ImGuiStyle & _Style)
{
  _Style.WindowPadding            = ImVec2(8.0f, 8.0f);
  _Style.FramePadding             = ImVec2(8.0f, 4.0f);
  _Style.ItemSpacing              = ImVec2(8.0f, 6.0f);
  _Style.ItemInnerSpacing         = ImVec2(6.0f, 4.0f);
  _Style.IndentSpacing            = 14.0f;
  _Style.ScrollbarSize            = 10.0f;
  _Style.GrabMinSize              = 10.0f;

  _Style.WindowRounding           = 0.0f;
  _Style.ChildRounding            = 4.0f;
  _Style.FrameRounding            = 4.0f;
  _Style.PopupRounding            = 4.0f;
  _Style.ScrollbarRounding        = 6.0f;
  _Style.GrabRounding             = 4.0f;
  _Style.TabRounding              = 4.0f;

  _Style.WindowBorderSize         = 1.0f;
  _Style.ChildBorderSize          = 1.0f;
  _Style.PopupBorderSize          = 1.0f;
  _Style.FrameBorderSize          = 0.0f;
  _Style.TabBorderSize            = 0.0f;
  _Style.TabBarBorderSize         = 1.0f;
  _Style.TabBarOverlineSize       = 2.0f;
  _Style.SeparatorTextBorderSize  = 1.0f;

  _Style.WindowMenuButtonPosition = ImGuiDir_None;
}

} // namespace

bool LoadEditorFont(const std::string & _FontPath, float _Scale)
{
  std::error_code ErrorCode;
  if (!std::filesystem::is_regular_file(_FontPath, ErrorCode))
    return false;

  return ImGui::GetIO().Fonts->AddFontFromFileTTF(_FontPath.c_str(), FontSize * _Scale) != nullptr;
}

void ApplyEditorStyle(float _Scale)
{
  ImGuiStyle & Style = ImGui::GetStyle();
  ImGui::StyleColorsDark(&Style);

  ApplyColors(Style);
  ApplyMetrics(Style);

  Style.ScaleAllSizes(_Scale);
}

} // namespace editor::ui
