#include "MaterialDocument.h"
#include "core/FunctionCommand.h"
#include "editor/assets/AssetRecord.h"
#include "editor/documents/AssetProperty.h"

#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreMaterialSerializer.h>
#include <OgrePass.h>
#include <OgreShaderGenerator.h>
#include <OgreTechnique.h>
#include <OgreTextureUnitState.h>

#include <fstream>
#include <type_traits>
#include <utility>

namespace editor
{

namespace
{

const std::string WorkingCopyPrefix = "OgreResourceEditor/WorkingCopy/";

std::string MakeWorkingCopyName(const std::string & _Name)
{
  static unsigned long long NextCopyIndex = 1;
  return WorkingCopyPrefix + std::to_string(NextCopyIndex++) + "/" + _Name;
}

void RemoveGeneratedTechniques(Ogre::Material & _Material)
{
  for (auto TechniqueIndex = _Material.getNumTechniques(); TechniqueIndex > 0; --TechniqueIndex)
  {
    const auto Index = static_cast<unsigned short>(TechniqueIndex - 1);
    if (_Material.getTechnique(Index)->getSchemeName() == Ogre::MSN_SHADERGEN)
      _Material.removeTechnique(Index);
  }
}

void ReleaseShaderBasedTechniques(const Ogre::Material & _Material)
{
  if (auto * ShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr())
    ShaderGenerator->removeAllShaderBasedTechniques(_Material);
}

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
// Constructor/Destructor
//

CMaterialDocument::CMaterialDocument(TAssetRecord _Record, const std::vector<Ogre::MaterialPtr> & _Materials, bool _HasExpandedFeatures):
  CAssetDocument(std::move(_Record), "Material"),
  m_HasExpandedFeatures(_HasExpandedFeatures)
{
  m_Materials.reserve(_Materials.size());

  for (const auto & Original : _Materials)
  {
    auto WorkingCopy = Original->clone(MakeWorkingCopyName(Original->getName()));
    WorkingCopy->_notifyOrigin({});

    RemoveGeneratedTechniques(*WorkingCopy);
    WorkingCopy->load();

    m_Materials.push_back({Original, std::move(WorkingCopy)});
  }
}

CMaterialDocument::~CMaterialDocument()
{
  auto * MaterialManager = Ogre::MaterialManager::getSingletonPtr();

  for (auto & Material : m_Materials)
  {
    ReleaseShaderBasedTechniques(*Material.WorkingCopy);

    if (MaterialManager)
      MaterialManager->remove(Material.WorkingCopy);
  }
}

//
// CAssetDocument
//

std::vector<TAssetProperty> CMaterialDocument::GetProperties() const
{
  return {{"Materials",     std::to_string(m_Materials.size())                               },
          {"Script layout", m_HasExpandedFeatures ? "Uses inheritance or variables" : "Plain"}};
}

bool CMaterialDocument::OnSave(std::string & _Error)
{
  CommitPreview();

  Ogre::MaterialSerializer Serializer;

  if (auto * ShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr())
    Serializer.addListener(ShaderGenerator->getMaterialSerializerListener());

  for (const auto & Material : m_Materials)
    Serializer.queueForExport(Material.WorkingCopy, false, false, Material.Original->getName());

  std::ofstream File(GetRecord().AbsolutePath, std::ios::binary | std::ios::trunc);
  File << Serializer.getQueuedAsString();
  File.close();

  if (!File)
  {
    _Error = "Could not write " + GetRecord().RelativePath.generic_string();
    return false;
  }

  for (auto & Material : m_Materials)
    ApplyWorkingCopy(Material);

  m_HasExpandedFeatures = false;

  return true;
}

//
// Interface
//

std::size_t CMaterialDocument::GetMaterialCount() const noexcept
{
  return m_Materials.size();
}

const Ogre::MaterialPtr & CMaterialDocument::GetWorkingCopy(std::size_t _Index) const
{
  return m_Materials.at(_Index).WorkingCopy;
}

const std::string & CMaterialDocument::GetMaterialName(std::size_t _Index) const
{
  return m_Materials.at(_Index).Original->getName();
}

bool CMaterialDocument::HasExpandedFeatures() const noexcept
{
  return m_HasExpandedFeatures;
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
      LockedDocument->m_ChangedSignal.Emit(Node);
    }
  };

  GetHistory().Execute(std::make_unique<core::CFunctionCommand>(GetCommandName(Preview.Before),
    [Apply, After = *After] { Apply(After); },
    [Apply, Before = Preview.Before] { Apply(Before); }));
}

//
// Signals
//

CMaterialDocument::TChangedSignal & CMaterialDocument::GetChangedSignal() noexcept
{
  return m_ChangedSignal;
}

//
// Implementation
//

void CMaterialDocument::ApplyWorkingCopy(TMaterialCopy & _Material)
{
  ReleaseShaderBasedTechniques(*_Material.Original);

  const auto Origin = _Material.Original->getOrigin();

  _Material.WorkingCopy->copyDetailsTo(_Material.Original);
  _Material.Original->_notifyOrigin(Origin);

  RemoveGeneratedTechniques(*_Material.Original);
  _Material.Original->compile();
}

Ogre::Technique * CMaterialDocument::FindTechnique(const TMaterialNode & _Node) const
{
  if (_Node.MaterialIndex < 0 || static_cast<std::size_t>(_Node.MaterialIndex) >= GetMaterialCount())
    return nullptr;

  const auto & Material = GetWorkingCopy(static_cast<std::size_t>(_Node.MaterialIndex));
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

  const auto & Material = GetWorkingCopy(static_cast<std::size_t>(_Node.MaterialIndex));
  ShaderGenerator->invalidateMaterial(Ogre::MSN_SHADERGEN, *Material);
}

} // namespace editor
