#include "SceneEditorController.h"
#include "core/ActionQueue.h"
#include "editor/assets/AssetType.h"
#include "editor/assets/AssetTypeInfo.h"
#include "editor/assets/AssetTypeRegistry.h"
#include "modules/scene/SceneDocument.h"
#include "modules/scene/drop/ISceneDropHandler.h"
#include "modules/scene/drop/SceneDrop.h"
#include "modules/scene/drop/SceneDropHandlers.h"
#include "modules/scene/model/SceneNodeProperties.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CSceneEditorController::CSceneEditorController(std::shared_ptr<core::CActionQueue> _ActionQueue, std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<CSceneDropHandlers> _DropHandlers):
  m_ActionQueue(std::move(_ActionQueue)),
  m_AssetTypes(std::move(_AssetTypes)),
  m_DropHandlers(std::move(_DropHandlers))
{
  // Empty
}

//
// ISceneEditorPresenter state
//

std::shared_ptr<const CSceneDocument> CSceneEditorController::GetDocument() const
{
  return m_Document.lock();
}

std::optional<std::string> CSceneEditorController::GetDropHint(const std::filesystem::path & _AssetPath, const TSceneDropTarget & _Target) const
{
  const auto Document = m_Document.lock();
  const auto Drop     = MakeDrop(_AssetPath, _Target);
  if (!Document || !Drop)
    return std::nullopt;

  const auto Handler = m_DropHandlers->FindHandler(*Drop, *Document);
  if (!Handler)
    return std::nullopt;

  return Handler->GetDropHint(*Drop);
}

//
// ISceneEditorPresenter actions
//

void CSceneEditorController::Select(const std::string & _Name)
{
  if (const auto Document = m_Document.lock())
    Document->Select(_Name);
}

void CSceneEditorController::CreateNode(const std::string & _ParentName)
{
  Post([_ParentName](CSceneDocument & _Document)
  {
    _Document.CreateNode(_ParentName, "Node", {});
  });
}

void CSceneEditorController::DeleteNode(const std::string & _Name)
{
  Post([_Name](CSceneDocument & _Document)
  {
    _Document.DeleteNode(_Name);
  });
}

void CSceneEditorController::MoveNode(const std::string & _Name, const std::string & _NewParentName)
{
  Post([_Name, _NewParentName](CSceneDocument & _Document)
  {
    _Document.MoveNode(_Name, _NewParentName);
  });
}

void CSceneEditorController::RenameNode(const std::string & _Name, const std::string & _NewName)
{
  Post([_Name, _NewName](CSceneDocument & _Document)
  {
    _Document.RenameNode(_Name, _NewName);
  });
}

void CSceneEditorController::DropAsset(const std::filesystem::path & _AssetPath, const TSceneDropTarget & _Target)
{
  auto Drop = MakeDrop(_AssetPath, _Target);
  if (!Drop)
    return;

  Post([DropHandlers = m_DropHandlers, Drop = std::move(*Drop)](CSceneDocument & _Document)
  {
    if (const auto Handler = DropHandlers->FindHandler(Drop, _Document))
      Handler->Drop(Drop, _Document);
  });
}

//
// Interface
//

void CSceneEditorController::SetDocument(const std::shared_ptr<CSceneDocument> & _Document)
{
  m_Document = _Document;
}

void CSceneEditorController::ResetDocument(const CSceneDocument & _Document)
{
  if (m_Document.lock().get() == &_Document)
    m_Document.reset();
}

//
// Implementation
//

void CSceneEditorController::Post(std::function<void(CSceneDocument &)> _Action)
{
  m_ActionQueue->Post([Document = m_Document, Action = std::move(_Action)]
  {
    if (const auto LockedDocument = Document.lock())
      Action(*LockedDocument);
  });
}

std::optional<TSceneDrop> CSceneEditorController::MakeDrop(const std::filesystem::path & _AssetPath, const TSceneDropTarget & _Target) const
{
  const auto AssetType = m_AssetTypes->FindByExtension(_AssetPath.extension().string());
  if (!AssetType)
    return std::nullopt;

  return TSceneDrop{_AssetPath, AssetType->GetInfo().Identifier, _Target};
}

} // namespace editor
