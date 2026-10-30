#include "MaterialInstance.h"
#include "editor/assets/AssetRecord.h"
#include "editor/workspaces/AssetProperty.h"

#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreMaterialSerializer.h>
#include <OgreShaderGenerator.h>
#include <OgreTechnique.h>

#include <fstream>
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

} // namespace

//
// Constructor/Destructor
//

CMaterialInstance::CMaterialInstance(TAssetRecord _Record, const std::vector<Ogre::MaterialPtr> & _Materials, bool _HasExpandedFeatures):
  CAssetInstance(std::move(_Record), "Material"),
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

CMaterialInstance::~CMaterialInstance()
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
// Interface
//

std::size_t CMaterialInstance::GetMaterialCount() const noexcept
{
  return m_Materials.size();
}

const Ogre::MaterialPtr & CMaterialInstance::GetWorkingCopy(std::size_t _Index) const
{
  return m_Materials.at(_Index).WorkingCopy;
}

const std::string & CMaterialInstance::GetMaterialName(std::size_t _Index) const
{
  return m_Materials.at(_Index).Original->getName();
}

bool CMaterialInstance::HasExpandedFeatures() const noexcept
{
  return m_HasExpandedFeatures;
}

bool CMaterialInstance::Save(std::string & _Error)
{
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
// CAssetInstance
//

std::vector<TAssetProperty> CMaterialInstance::GetProperties() const
{
  return {{"Materials",     std::to_string(m_Materials.size())                               },
          {"Script layout", m_HasExpandedFeatures ? "Uses inheritance or variables" : "Plain"}};
}

//
// Implementation
//

void CMaterialInstance::ApplyWorkingCopy(TMaterialCopy & _Material)
{
  ReleaseShaderBasedTechniques(*_Material.Original);

  const auto Origin = _Material.Original->getOrigin();

  _Material.WorkingCopy->copyDetailsTo(_Material.Original);
  _Material.Original->_notifyOrigin(Origin);

  RemoveGeneratedTechniques(*_Material.Original);
  _Material.Original->compile();
}

} // namespace editor
