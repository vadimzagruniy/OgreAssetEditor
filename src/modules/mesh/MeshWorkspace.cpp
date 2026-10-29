#include "MeshWorkspace.h"
#include "MeshInstance.h"
#include "MeshPreview.h"
#include "OrbitCamera.h"

#include <OgreTexture.h>
#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CMeshWorkspace::CMeshWorkspace(std::unique_ptr<CMeshInstance> _Instance):
  CEditorWorkspace(std::move(_Instance)),
  m_Preview(std::make_unique<CMeshPreview>(static_cast<CMeshInstance *>(GetInstance())->GetMesh()))
{
  // Empty
}

CMeshWorkspace::~CMeshWorkspace() = default;

//
// CEditorWorkspace
//

void CMeshWorkspace::OnActivated()
{
  m_Preview->SetActive(true);
}

void CMeshWorkspace::OnDeactivated()
{
  m_Preview->SetActive(false);
}

void CMeshWorkspace::OnGui()
{
  const ImVec2 Available = ImGui::GetContentRegionAvail();
  if (Available.x < 1.0f || Available.y < 1.0f)
    return;

  m_Preview->Resize(static_cast<unsigned int>(Available.x), static_cast<unsigned int>(Available.y));

  const ImVec2 Origin = ImGui::GetCursorScreenPos();

  ImGui::InvisibleButton("##MeshViewport", Available, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
  HandleViewportInput();

  if (const auto & Texture = m_Preview->GetTexture())
  {
    const auto TextureId = reinterpret_cast<ImTextureID>(Texture->getHandle());
    ImGui::GetWindowDrawList()->AddImage(TextureId, Origin, ImVec2(Origin.x + Available.x, Origin.y + Available.y));
  }
}

//
// Implementation
//

void CMeshWorkspace::HandleViewportInput()
{
  const ImGuiIO & InputState = ImGui::GetIO();
  auto &          Camera     = m_Preview->GetCamera();

  if (ImGui::IsItemActive())
  {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
      Camera.Rotate(InputState.MouseDelta.x, InputState.MouseDelta.y);
    else
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
      Camera.Pan(InputState.MouseDelta.x, InputState.MouseDelta.y);
  }

  if (ImGui::IsItemHovered() && InputState.MouseWheel != 0.0f)
    Camera.Zoom(InputState.MouseWheel);
}

} // namespace editor
