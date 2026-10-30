#include "EditorStyle.h"

#include <imgui.h>

#include <filesystem>

namespace editor::ui
{

namespace
{

constexpr float FontSize = 15.0f;

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

  Style.WindowPadding     = ImVec2(8.0f, 8.0f);
  Style.FramePadding      = ImVec2(6.0f, 4.0f);
  Style.ItemSpacing       = ImVec2(8.0f, 5.0f);
  Style.WindowRounding    = 0.0f;
  Style.ChildRounding     = 2.0f;
  Style.FrameRounding     = 3.0f;
  Style.PopupRounding     = 3.0f;
  Style.ScrollbarRounding = 3.0f;
  Style.GrabRounding      = 3.0f;
  Style.TabRounding       = 3.0f;
  Style.WindowBorderSize  = 1.0f;
  Style.FrameBorderSize   = 0.0f;
  Style.TabBorderSize     = 0.0f;

  Style.ScaleAllSizes(_Scale);
}

} // namespace editor::ui
