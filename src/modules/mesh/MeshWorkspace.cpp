#include "MeshWorkspace.h"
#include "MeshInstance.h"
#include "editor/render/PreviewScene.h"
#include "editor/render/PreviewViewport.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CMeshWorkspace::CMeshWorkspace(std::unique_ptr<CMeshInstance> _Instance):
  CEditorWorkspace(std::move(_Instance)),
  m_Preview(std::make_unique<CPreviewScene>())
{
  m_Preview->SetMesh(static_cast<CMeshInstance *>(GetInstance())->GetMesh());
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
  DrawPreviewViewport("##MeshViewport", *m_Preview);
}

} // namespace editor
