#pragma once

#include "MaterialNode.h"
#include "MaterialState.h"

#include <OgrePrerequisites.h>

#include <memory>
#include <optional>
#include <variant>

namespace editor
{

namespace core
{
class CUndoStack;
} // namespace core

class CMaterialInstance;

class CMaterialDocument final:
  public std::enable_shared_from_this<CMaterialDocument>
{
public: // Constructor

  CMaterialDocument(CMaterialInstance & _Instance, core::CUndoStack & _UndoStack);

public: // Interface

  const CMaterialInstance & GetInstance() const noexcept;

  const TMaterialNode & GetSelection() const noexcept;

  void Select(const TMaterialNode & _Node);

  static bool IsShaderGenerated(const Ogre::Technique & _Technique);

  std::optional<TPassState> GetPassState(const TMaterialNode & _Node) const;

  std::optional<TTextureUnitState> GetTextureUnitState(const TMaterialNode & _Node) const;

  void Preview(const TMaterialNode & _Node, const TPassState & _State);

  void Preview(const TMaterialNode & _Node, const TTextureUnitState & _State);

  void CommitPreview();

private: // Types

  using TState = std::variant<TPassState, TTextureUnitState>;

  struct TPreview
  {
    TMaterialNode Node;
    TState        Before;
  };

private: // Implementation

  Ogre::Technique * FindTechnique(const TMaterialNode & _Node) const;

  Ogre::Pass * FindPass(const TMaterialNode & _Node) const;

  Ogre::TextureUnitState * FindTextureUnit(const TMaterialNode & _Node) const;

  std::optional<TState> ReadState(const TMaterialNode & _Node, const TState & _Kind) const;

  void PreviewState(const TMaterialNode & _Node, const TState & _State);

  void ApplyState(const TMaterialNode & _Node, const TState & _State);

  void InvalidateShaders(const TMaterialNode & _Node) const;

private: // Attributes

  CMaterialInstance &     m_Instance;
  core::CUndoStack &      m_UndoStack;
  TMaterialNode           m_Selection;
  std::optional<TPreview> m_Preview;
};

} // namespace editor
