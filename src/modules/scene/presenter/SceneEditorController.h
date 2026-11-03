#pragma once

#include "ISceneEditorPresenter.h"

#include <functional>
#include <memory>

namespace editor
{

namespace core
{
class CActionQueue;
} // namespace core

class CAssetTypeRegistry;
class CSceneDropHandlers;
struct TSceneDrop;

class CSceneEditorController final:
  public ISceneEditorPresenter
{
public: // Constructor

  CSceneEditorController(std::shared_ptr<core::CActionQueue> _ActionQueue, std::shared_ptr<CAssetTypeRegistry> _AssetTypes, std::shared_ptr<CSceneDropHandlers> _DropHandlers);

public: // ISceneEditorPresenter state

  std::shared_ptr<const CSceneDocument> GetDocument() const override;

  std::optional<std::string> GetDropHint(const std::filesystem::path & _AssetPath, const TSceneDropTarget & _Target) const override;

public: // ISceneEditorPresenter actions

  void Select(const std::string & _Name) override;

  void CreateNode(const std::string & _ParentName) override;

  void DeleteNode(const std::string & _Name) override;

  void MoveNode(const std::string & _Name, const std::string & _NewParentName) override;

  void RenameNode(const std::string & _Name, const std::string & _NewName) override;

  void DropAsset(const std::filesystem::path & _AssetPath, const TSceneDropTarget & _Target) override;

public: // Interface

  void SetDocument(const std::shared_ptr<CSceneDocument> & _Document);

  void ResetDocument(const CSceneDocument & _Document);

private: // Implementation

  void Post(std::function<void(CSceneDocument &)> _Action);

  std::optional<TSceneDrop> MakeDrop(const std::filesystem::path & _AssetPath, const TSceneDropTarget & _Target) const;

private: // Attributes

  std::shared_ptr<core::CActionQueue> m_ActionQueue;
  std::shared_ptr<CAssetTypeRegistry> m_AssetTypes;
  std::shared_ptr<CSceneDropHandlers> m_DropHandlers;
  std::weak_ptr<CSceneDocument>       m_Document;
};

} // namespace editor
