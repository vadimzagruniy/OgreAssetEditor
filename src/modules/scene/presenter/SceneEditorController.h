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

class CSceneEditorController final:
  public ISceneEditorPresenter
{
public: // Constructor

  explicit CSceneEditorController(std::shared_ptr<core::CActionQueue> _ActionQueue);

public: // ISceneEditorPresenter state

  std::shared_ptr<const CSceneDocument> GetDocument() const override;

public: // ISceneEditorPresenter actions

  void Select(const std::string & _Name) override;

  void CreateNode(const std::string & _ParentName) override;

  void DeleteNode(const std::string & _Name) override;

  void MoveNode(const std::string & _Name, const std::string & _NewParentName) override;

  void RenameNode(const std::string & _Name, const std::string & _NewName) override;

public: // Interface

  void SetDocument(const std::shared_ptr<CSceneDocument> & _Document);

  void ResetDocument(const CSceneDocument & _Document);

private: // Implementation

  void Post(std::function<void(CSceneDocument &)> _Action);

private: // Attributes

  std::shared_ptr<core::CActionQueue> m_ActionQueue;
  std::weak_ptr<CSceneDocument>       m_Document;
};

} // namespace editor
