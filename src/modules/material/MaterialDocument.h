#pragma once

#include "MaterialNode.h"
#include "MaterialState.h"
#include "core/Signal.h"
#include "editor/documents/AssetDocument.h"

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace editor
{

class CMaterialDocument final:
  public CAssetDocument,
  public std::enable_shared_from_this<CMaterialDocument>
{
public: // Types

  using TChangedSignal = core::CSignal<const TMaterialNode &>;

public: // Constructor/Destructor

  CMaterialDocument(TAssetRecord _Record, const std::vector<Ogre::MaterialPtr> & _Materials, bool _HasExpandedFeatures);

  ~CMaterialDocument() override;

public: // CAssetDocument

  std::vector<TAssetProperty> GetProperties() const override;

protected: // CAssetDocument

  bool OnSave(std::string & _Error) override;

public: // Interface

  std::size_t GetMaterialCount() const noexcept;

  const Ogre::MaterialPtr & GetWorkingCopy(std::size_t _Index) const;

  const std::string & GetMaterialName(std::size_t _Index) const;

  bool HasExpandedFeatures() const noexcept;

  static bool IsShaderGenerated(const Ogre::Technique & _Technique);

  std::optional<TPassState> GetPassState(const TMaterialNode & _Node) const;

  std::optional<TTextureUnitState> GetTextureUnitState(const TMaterialNode & _Node) const;

  void Preview(const TMaterialNode & _Node, const TPassState & _State);

  void Preview(const TMaterialNode & _Node, const TTextureUnitState & _State);

  void CommitPreview() override;

public: // Signals

  TChangedSignal & GetChangedSignal() noexcept;

private: // Types

  using TState = std::variant<TPassState, TTextureUnitState>;

  struct TMaterialCopy
  {
    Ogre::MaterialPtr Original;
    Ogre::MaterialPtr WorkingCopy;
  };

  struct TPreview
  {
    TMaterialNode Node;
    TState        Before;
  };

private: // Implementation

  static void ApplyWorkingCopy(TMaterialCopy & _Material);

  Ogre::Technique * FindTechnique(const TMaterialNode & _Node) const;

  Ogre::Pass * FindPass(const TMaterialNode & _Node) const;

  Ogre::TextureUnitState * FindTextureUnit(const TMaterialNode & _Node) const;

  std::optional<TState> ReadState(const TMaterialNode & _Node, const TState & _Kind) const;

  void PreviewState(const TMaterialNode & _Node, const TState & _State);

  void ApplyState(const TMaterialNode & _Node, const TState & _State);

  void InvalidateShaders(const TMaterialNode & _Node) const;

private: // Attributes

  std::vector<TMaterialCopy> m_Materials;
  std::optional<TPreview>    m_Preview;
  TChangedSignal             m_ChangedSignal;
  bool                       m_HasExpandedFeatures;
};

} // namespace editor
