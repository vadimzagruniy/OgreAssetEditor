#include "PreviewViewport.h"
#include "OrbitCamera.h"
#include "PreviewScene.h"

#include <OgreTexture.h>
#include <imgui.h>

namespace editor
{

namespace
{

void HandleCameraInput(COrbitCamera & _Camera)
{
  const ImGuiIO & InputState = ImGui::GetIO();

  if (ImGui::IsItemActive())
  {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
      _Camera.Rotate(InputState.MouseDelta.x, InputState.MouseDelta.y);
    else
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
      _Camera.Pan(InputState.MouseDelta.x, InputState.MouseDelta.y);
  }

  if (ImGui::IsItemHovered() && InputState.MouseWheel != 0.0f)
    _Camera.Zoom(InputState.MouseWheel);
}

} // namespace

void DrawPreviewViewport(const char * _Id, CPreviewScene & _Scene)
{
  const ImVec2 Available = ImGui::GetContentRegionAvail();
  if (Available.x < 1.0f || Available.y < 1.0f)
    return;

  _Scene.Resize(static_cast<unsigned int>(Available.x), static_cast<unsigned int>(Available.y));

  const ImVec2 Origin = ImGui::GetCursorScreenPos();

  ImGui::InvisibleButton(_Id, Available, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
  HandleCameraInput(_Scene.GetCamera());

  if (const auto & Texture = _Scene.GetTexture())
  {
    const auto TextureId = reinterpret_cast<ImTextureID>(Texture->getHandle());
    ImGui::GetWindowDrawList()->AddImage(TextureId, Origin, ImVec2(Origin.x + Available.x, Origin.y + Available.y));
  }
}

} // namespace editor
