#include "SceneWorkspace.h"
#include "SceneDocument.h"
#include "SceneInstance.h"
#include "editor/render/RenderScene.h"
#include "editor/viewport/Viewport.h"
#include "presenter/SceneEditorController.h"
#include "render/SceneSync.h"
#include "views/SceneOutlinerWindow.h"

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CSceneWorkspace::CSceneWorkspace(std::unique_ptr<CSceneInstance> _Instance, std::shared_ptr<CSceneEditorController> _Controller, std::shared_ptr<core::CLog> _Log):
  CEditorWorkspace(std::move(_Instance)),
  m_Controller(std::move(_Controller)),
  m_Document(std::make_shared<CSceneDocument>(GetScene(), GetUndoStack())),
  m_RenderScene(std::make_unique<CRenderScene>()),
  m_Sync(std::make_unique<CSceneSync>(*m_RenderScene, std::move(_Log))),
  m_Viewport(std::make_unique<CViewport>(*m_RenderScene))
{
  m_Sync->Update(GetScene());
  m_RenderScene->MarkContentReplaced();

  m_Viewport->OnPick([this](const Ogre::Ray & _Ray)
  {
    m_Document->Select(m_Sync->Pick(_Ray));
  });
}

CSceneWorkspace::~CSceneWorkspace() = default;

//
// CEditorWorkspace
//

std::vector<core::TTypeKey> CSceneWorkspace::GetToolBoxes() const
{
  return {core::TypeOf<CSceneOutlinerWindow>()};
}

void CSceneWorkspace::OnActivated()
{
  m_Controller->SetDocument(m_Document);
  m_RenderScene->SetActive(true);
}

void CSceneWorkspace::OnDeactivated()
{
  m_Controller->ResetDocument(*m_Document);
  m_RenderScene->SetActive(false);
}

void CSceneWorkspace::OnGui()
{
  m_Sync->Update(GetScene());
  m_Sync->SetHighlightedNode(m_Document->GetSelection());

  m_Viewport->Draw("##SceneViewport");
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
