#include "MeshWorkspace.h"
#include "MeshInstance.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor
//

CMeshWorkspace::CMeshWorkspace(std::unique_ptr<CMeshInstance> _Instance):
  CEditorWorkspace(std::move(_Instance))
{
  // Empty
}

//
// CEditorWorkspace
//

void CMeshWorkspace::OnGui()
{
  ImGui::TextUnformatted("OGRE mesh resource loaded.");
  ImGui::TextWrapped("The mesh viewer module owns this resource workspace. A scene preview can be added here without modifying the editor core.");
}

} // namespace editor
