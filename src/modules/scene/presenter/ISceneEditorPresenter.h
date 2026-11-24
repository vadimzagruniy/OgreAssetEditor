#pragma once

#include "modules/scene/drop/SceneDropTarget.h"
#include "modules/scene/model/SceneNodeTemplate.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace editor
{

class CSceneDocument;

class ISceneEditorPresenter
{
public: // Destructor

  virtual ~ISceneEditorPresenter();

public: // State

  virtual std::shared_ptr<const CSceneDocument> GetDocument() const = 0;

  virtual std::string GetSelection() const = 0;

  virtual bool CanPaste() const = 0;

  virtual const std::vector<TSceneNodeTemplate> & GetTemplates() const = 0;

  virtual std::optional<std::string> GetDropHint(const std::filesystem::path & _AssetPath, const TSceneDropTarget & _Target) const = 0;

public: // Actions

  virtual void Select(const std::string & _Name) = 0;

  virtual void CreateFromTemplate(const std::string & _TemplateId, const TSceneDropTarget & _Target) = 0;

  virtual void DeleteNode(const std::string & _Name) = 0;

  virtual void CopyNode(const std::string & _Name) = 0;

  virtual void PasteNode(const std::string & _TargetName, bool _IsChild) = 0;

  virtual void DuplicateNode(const std::string & _Name) = 0;

  virtual void MoveNode(const std::string & _Name, const std::string & _NewParentName) = 0;

  virtual void RenameNode(const std::string & _Name, const std::string & _NewName) = 0;

  virtual void DropAsset(const std::filesystem::path & _AssetPath, const TSceneDropTarget & _Target) = 0;
};

} // namespace editor
