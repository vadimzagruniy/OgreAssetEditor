#pragma once

#include <memory>

namespace editor
{

class CMaterialDocument;
struct TMaterialNode;
struct TPassState;
struct TTextureUnitState;

class IMaterialEditorPresenter
{
public: // Destructor

  virtual ~IMaterialEditorPresenter();

public: // State

  virtual std::shared_ptr<const CMaterialDocument> GetDocument() const = 0;

public: // Actions

  virtual void Select(const TMaterialNode & _Node) = 0;

  virtual void Preview(const TMaterialNode & _Node, const TPassState & _State) = 0;

  virtual void Preview(const TMaterialNode & _Node, const TTextureUnitState & _State) = 0;

  virtual void CommitEdit() = 0;
};

} // namespace editor
