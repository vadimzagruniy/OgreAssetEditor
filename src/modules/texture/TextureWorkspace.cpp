#include "TextureWorkspace.h"
#include "TextureInstance.h"
#include "editor/assets/AssetRecord.h"

#include <OgreTexture.h>
#include <imgui.h>

#include <algorithm>
#include <utility>

namespace editor
{

//
// Constructor
//

CTextureWorkspace::CTextureWorkspace(std::unique_ptr<CTextureInstance> _Instance):
  CEditorWorkspace(std::move(_Instance))
{
  // Empty
}

//
// CEditorWorkspace
//

void CTextureWorkspace::OnGui()
{
  const auto * Instance = static_cast<CTextureInstance *>(GetInstance());
  const auto & Texture  = Instance->GetTexture();

  ImGui::TextUnformatted(Instance->GetRecord().RelativePath.generic_string().c_str());

  const float TextureWidth  = static_cast<float>(Texture->getWidth());
  const float TextureHeight = static_cast<float>(Texture->getHeight());

  if (TextureWidth <= 0 || TextureHeight <= 0)
    return;

  const auto  Available = ImGui::GetContentRegionAvail();
  const float Scale     = std::min(Available.x / TextureWidth, Available.y / TextureHeight);

  ImGui::Image(reinterpret_cast<ImTextureID>(Texture->getHandle()), ImVec2(TextureWidth * Scale, TextureHeight * Scale));
}

} // namespace editor
