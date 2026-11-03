#include "SceneWorkspace.h"
#include "SceneInstance.h"

#include <imgui.h>

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CSceneWorkspace::CSceneWorkspace(std::unique_ptr<CSceneInstance> _Instance):
  CEditorWorkspace(std::move(_Instance))
{
  // Empty
}

CSceneWorkspace::~CSceneWorkspace() = default;

//
// CEditorWorkspace
//

void CSceneWorkspace::OnGui()
{
  ImGui::TextDisabled("Scene viewport is not implemented yet.");
}

bool CSceneWorkspace::OnSave(std::string & _Error)
{
  return GetScene().Save(_Error);
}

//
// Implementation
//

CSceneInstance & CSceneWorkspace::GetScene() const
{
  return *static_cast<CSceneInstance *>(GetInstance());
}

} // namespace editor
