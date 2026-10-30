#include "MaterialDocument.h"
#include "MaterialInstance.h"
#include "core/FunctionCommand.h"
#include "core/UndoStack.h"

#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreShaderGenerator.h>
#include <OgreTechnique.h>
#include <OgreTextureUnitState.h>

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

namespace editor
{

namespace
{

TPassState ReadPassState(const Ogre::Pass & _Pass)
{
  TPassState State;

  State.Ambient             = _Pass.getAmbient();
  State.Diffuse             = _Pass.getDiffuse();
  State.Specular            = _Pass.getSpecular();
  State.Emissive            = _Pass.getSelfIllumination();
  State.Shininess           = _Pass.getShininess();
  State.SourceBlend         = _Pass.getSourceBlendFactor();
  State.DestBlend           = _Pass.getDestBlendFactor();
  State.IsDepthWriteEnabled = _Pass.getDepthWriteEnabled();
  State.Culling             = _Pass.getCullingMode();

  return State;
}

void WritePassState(Ogre::Pass & _Pass, const TPassState & _State)
{
  _Pass.setAmbient(_State.Ambient);
  _Pass.setDiffuse(_State.Diffuse);
  _Pass.setSpecular(_State.Specular);
  _Pass.setSelfIllumination(_State.Emissive);
  _Pass.setShininess(_State.Shininess);
  _Pass.setSceneBlending(_State.SourceBlend, _State.DestBlend);
  _Pass.setDepthWriteEnabled(_State.IsDepthWriteEnabled);
  _Pass.setCullingMode(_State.Culling);
}

TTextureUnitState ReadTextureUnitState(const Ogre::TextureUnitState & _TextureUnit)
{
  const auto & Addressing = _TextureUnit.getTextureAddressingMode();

  return {_TextureUnit.getTextureName(), Addressing.u, Addressing.v, Addressing.w};
}

void WriteTextureUnitState(Ogre::TextureUnitState & _TextureUnit, const TTextureUnitState & _State)
{
  if (_TextureUnit.getTextureName() != _State.TextureName)
    _TextureUnit.setTextureName(_State.TextureName);

  _TextureUnit.setTextureAddressingMode(_State.AddressU, _State.AddressV, _State.AddressW);
}

const char * GetCommandName(const std::variant<TPassState, TTextureUnitState> & _State)
{
  return std::holds_alternative<TPassState>(_State) ? "Edit Pass" : "Edit Texture Unit";
}

} // namespace

//
// Constructor
//

CMaterialDocument::CMaterialDocument(CMaterialInstance & _Instance, core::CUndoStack & _UndoStack):
  m_Instance(_Instance),
  m_UndoStack(_UndoStack),
  m_Selection{_Instance.GetMaterialCount() > 0 ? 0 : -1}
{
  // Empty
}

//
// Interface
//

const CMaterialInstance & CMaterialDocument::GetInstance() const noexcept
{
  return m_Instance;
}

const TMaterialNode & CMaterialDocument::GetSelection() const noexcept
{
  return m_Selection;
}

void CMaterialDocument::Select(const TMaterialNode & _Node)
{
  CommitPreview();

  m_Selection = _Node;
}

bool CMaterialDocument::IsShaderGenerated(const Ogre::Technique & _Technique)
{
  return _Technique.getSchemeName() == Ogre::MSN_SHADERGEN;
}

std::optional<TPassState> CMaterialDocument::GetPassState(const TMaterialNode & _Node) const
{
  if (_Node.TextureUnitIndex >= 0)
    return std::nullopt;

  const auto * Pass = FindPass(_Node);
  if (!Pass)
    return std::nullopt;

  return ReadPassState(*Pass);
}

std::optional<TTextureUnitState> CMaterialDocument::GetTextureUnitState(const TMaterialNode & _Node) const
{
  const auto * TextureUnit = FindTextureUnit(_Node);
  if (!TextureUnit)
    return std::nullopt;

  return ReadTextureUnitState(*TextureUnit);
}

void CMaterialDocument::Preview(const TMaterialNode & _Node, const TPassState & _State)
{
  PreviewState(_Node, _State);
}

void CMaterialDocument::Preview(const TMaterialNode & _Node, const TTextureUnitState & _State)
{
  PreviewState(_Node, _State);
}

void CMaterialDocument::CommitPreview()
{
  if (!m_Preview)
    return;

  const TPreview Preview = std::move(*m_Preview);
  m_Preview.reset();

  const auto After = ReadState(Preview.Node, Preview.Before);
  if (!After || *After == Preview.Before)
    return;

  auto Apply = [Document = weak_from_this(), Node = Preview.Node](const TState & _State)
  {
    if (const auto LockedDocument = Document.lock())
    {
      LockedDocument->m_Preview.reset();
      LockedDocument->ApplyState(Node, _State);
      LockedDocument->m_Selection = Node;
    }
  };

  m_UndoStack.Execute(std::make_unique<core::CFunctionCommand>(GetCommandName(Preview.Before),
    [Apply, After = *After] { Apply(After); },
    [Apply, Before = Preview.Before] { Apply(Before); }));
}

//
// Implementation
//

Ogre::Technique * CMaterialDocument::FindTechnique(const TMaterialNode & _Node) const
{
  if (_Node.MaterialIndex < 0 || static_cast<std::size_t>(_Node.MaterialIndex) >= m_Instance.GetMaterialCount())
    return nullptr;

  const auto & Material = m_Instance.GetWorkingCopy(static_cast<std::size_t>(_Node.MaterialIndex));
  if (_Node.TechniqueIndex < 0 || static_cast<std::size_t>(_Node.TechniqueIndex) >= Material->getNumTechniques())
    return nullptr;

  auto * Technique = Material->getTechnique(static_cast<std::size_t>(_Node.TechniqueIndex));
  return IsShaderGenerated(*Technique) ? nullptr : Technique;
}

Ogre::Pass * CMaterialDocument::FindPass(const TMaterialNode & _Node) const
{
  const auto * Technique = FindTechnique(_Node);
  if (!Technique || _Node.PassIndex < 0 || static_cast<std::size_t>(_Node.PassIndex) >= Technique->getNumPasses())
    return nullptr;

  return Technique->getPass(static_cast<std::size_t>(_Node.PassIndex));
}

Ogre::TextureUnitState * CMaterialDocument::FindTextureUnit(const TMaterialNode & _Node) const
{
  const auto * Pass = FindPass(_Node);
  if (!Pass || _Node.TextureUnitIndex < 0 || static_cast<std::size_t>(_Node.TextureUnitIndex) >= Pass->getNumTextureUnitStates())
    return nullptr;

  return Pass->getTextureUnitState(static_cast<std::size_t>(_Node.TextureUnitIndex));
}

std::optional<CMaterialDocument::TState> CMaterialDocument::ReadState(const TMaterialNode & _Node, const TState & _Kind) const
{
  if (std::holds_alternative<TPassState>(_Kind))
  {
    if (auto State = GetPassState(_Node))
      return TState(std::move(*State));

    return std::nullopt;
  }

  if (auto State = GetTextureUnitState(_Node))
    return TState(std::move(*State));

  return std::nullopt;
}

void CMaterialDocument::PreviewState(const TMaterialNode & _Node, const TState & _State)
{
  if (m_Preview && !(m_Preview->Node == _Node && m_Preview->Before.index() == _State.index()))
    CommitPreview();

  if (!m_Preview)
  {
    auto Before = ReadState(_Node, _State);
    if (!Before)
      return;

    m_Preview = TPreview{_Node, std::move(*Before)};
  }

  ApplyState(_Node, _State);
}

void CMaterialDocument::ApplyState(const TMaterialNode & _Node, const TState & _State)
{
  std::visit([this, &_Node](const auto & _Value)
  {
    using TValue = std::decay_t<decltype(_Value)>;

    if constexpr (std::is_same_v<TValue, TPassState>)
    {
      if (auto * Pass = FindPass(_Node))
        WritePassState(*Pass, _Value);
    }
    else
    {
      if (auto * TextureUnit = FindTextureUnit(_Node))
        WriteTextureUnitState(*TextureUnit, _Value);
    }
  }, _State);

  InvalidateShaders(_Node);
}

void CMaterialDocument::InvalidateShaders(const TMaterialNode & _Node) const
{
  auto * ShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
  if (!ShaderGenerator || _Node.MaterialIndex < 0)
    return;

  const auto & Material = m_Instance.GetWorkingCopy(static_cast<std::size_t>(_Node.MaterialIndex));
  ShaderGenerator->invalidateMaterial(Ogre::MSN_SHADERGEN, *Material);
}

} // namespace editor
