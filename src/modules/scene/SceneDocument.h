#pragma once

#include <functional>
#include <memory>
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

  void CreateNode(const std::string & _ParentName);

  void DeleteNode(const std::string & _Name);

  void MoveNode(const std::string & _Name, const std::string & _NewParentName);

  void RenameNode(const std::string & _Name, const std::string & _NewName);

private: // Types

  using TEdit = std::function<void(CSceneDocument &)>;

private: // Implementation

  void ExecuteCommand(std::string _CommandName, TEdit _Execute, TEdit _Undo);

private: // Attributes

  CSceneInstance &   m_Instance;
  core::CUndoStack & m_UndoStack;
  std::string        m_Selection;
};

} // namespace editor
