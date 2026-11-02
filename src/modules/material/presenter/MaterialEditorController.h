#pragma once

#include "IMaterialEditorPresenter.h"

#include <functional>
#include <memory>

namespace editor
{

namespace core
{
class CActionQueue;
} // namespace core

class CMaterialDocument;

class CMaterialEditorController final:
  public IMaterialEditorPresenter
{
public: // Constructor

  explicit CMaterialEditorController(std::shared_ptr<core::CActionQueue> _ActionQueue);

public: // IMaterialEditorPresenter state

  std::shared_ptr<const CMaterialDocument> GetDocument() const override;

public: // IMaterialEditorPresenter actions

  void Select(const TMaterialNode & _Node) override;

  void Preview(const TMaterialNode & _Node, const TPassState & _State) override;

  void Preview(const TMaterialNode & _Node, const TTextureUnitState & _State) override;

  void CommitEdit() override;

public: // Interface

  void SetDocument(const std::shared_ptr<CMaterialDocument> & _Document);

  void ResetDocument(const CMaterialDocument & _Document);

private: // Implementation

  void Post(std::function<void(CMaterialDocument &)> _Action);

private: // Attributes

  std::shared_ptr<core::CActionQueue> m_ActionQueue;
  std::weak_ptr<CMaterialDocument>    m_Document;
};

} // namespace editor
