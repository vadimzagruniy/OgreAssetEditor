#pragma once

#include "model/SceneNodeProperties.h"

#include <OgrePrerequisites.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace editor
{

namespace core
{
class CUndoStack;
} // namespace core

class CSceneInstance;

class CSceneDocument final:
  public std::enable_shared_from_this<CSceneDocument>
{
public: // Constructor

  CSceneDocument(CSceneInstance & _Instance, core::CUndoStack & _UndoStack);

public: // Interface

  const CSceneInstance & GetInstance() const noexcept;

  std::string GetSelection() const;

  void Select(const std::string & _Name);

  bool CanRename(const std::string & _Name, const std::string & _NewName, std::string & _Error) const;

  bool CanMove(const std::string & _Name, const std::string & _NewParentName) const;

  void CreateNode(const std::string & _ParentName, const std::string & _BaseName, const TSceneNodeProperties & _Properties);

  void DeleteNode(const std::string & _Name);

  void MoveNode(const std::string & _Name, const std::string & _NewParentName);

  void RenameNode(const std::string & _Name, const std::string & _NewName);

  void PreviewWorldTransform(const std::string & _Name, const Ogre::Affine3 & _World);

  void CommitPreview();

  void CancelPreview();

private: // Types

  using TEdit = std::function<void(CSceneDocument &)>;

  struct TPreview
  {
    std::string          Name;
    TSceneNodeProperties Before;
  };

private: // Implementation

  void ExecuteCommand(std::string _CommandName, TEdit _Execute, TEdit _Undo);

private: // Attributes

  CSceneInstance &        m_Instance;
  core::CUndoStack &      m_UndoStack;
  std::string             m_Selection;
  std::optional<TPreview> m_Preview;
};

} // namespace editor
