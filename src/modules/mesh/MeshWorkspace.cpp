#include "MeshWorkspace.h"
#include "MeshDocument.h"
#include "editor/render/PreviewScene.h"
#include "editor/render/RenderScene.h"
#include "editor/viewport/Viewport.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CMeshWorkspace::CMeshWorkspace(const std::shared_ptr<CMeshDocument> & _Document):
  CEditorWorkspace(_Document),
  m_Preview(std::make_unique<CPreviewScene>()),
  m_Viewport(std::make_unique<CViewport>(m_Preview->GetRenderScene()))
{
  m_Preview->SetMesh(_Document->GetMesh());
}

CMeshWorkspace::~CMeshWorkspace() = default;

//
// CEditorWorkspace
//

void CMeshWorkspace::OnActivated()
{
  m_Preview->GetRenderScene().SetActive(true);
}

void CMeshWorkspace::OnDeactivated()
{
  m_Preview->GetRenderScene().SetActive(false);
}

void CMeshWorkspace::OnGui()
{
  m_Viewport->Draw("##MeshViewport");
}

} // namespace editor
