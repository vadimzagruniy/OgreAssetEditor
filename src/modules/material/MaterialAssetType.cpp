#include "MaterialAssetType.h"
#include "MaterialInstance.h"
#include "MaterialWorkspace.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetTypeInfo.h"

#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreScriptCompiler.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

namespace editor
{

namespace
{

bool IsScriptOrigin(const std::string & _Origin, const std::filesystem::path & _RelativePath)
{
  if (_Origin == _RelativePath.generic_string())
    return true;

  return _Origin.find('/') == std::string::npos && _Origin == _RelativePath.filename().string();
}

std::vector<Ogre::MaterialPtr> FindScriptMaterials(const std::filesystem::path & _RelativePath, const std::string & _ResourceGroup)
{
  std::vector<Ogre::MaterialPtr> Materials;

  auto Resources = Ogre::MaterialManager::getSingleton().getResourceIterator();
  while (Resources.hasMoreElements())
  {
    auto Material = std::static_pointer_cast<Ogre::Material>(Resources.getNext());
    if (Material->getGroup() == _ResourceGroup && IsScriptOrigin(Material->getOrigin(), _RelativePath))
      Materials.push_back(std::move(Material));
  }

  return Materials;
}

bool ParseScript(const std::filesystem::path & _RelativePath, const std::string & _ResourceGroup, std::string & _Error)
{
  auto & ResourceGroups = Ogre::ResourceGroupManager::getSingleton();

  const auto ResourceName = _RelativePath.generic_string();
  if (!ResourceGroups.resourceExists(_ResourceGroup, ResourceName))
  {
    _Error = "OGRE cannot find " + ResourceName + " in the project resources.";
    return false;
  }

  auto Stream = ResourceGroups.openResource(ResourceName, _ResourceGroup);
  Ogre::ScriptCompilerManager::getSingleton().parseScript(Stream, _ResourceGroup);

  return true;
}

bool HasExpandedFeatures(const std::string & _Script)
{
  std::istringstream Lines(_Script);
  std::string        Line;

  while (std::getline(Lines, Line))
  {
    Line = Line.substr(0, Line.find("//"));

    std::istringstream Tokens(Line);
    std::string        Keyword;
    if (!(Tokens >> Keyword))
      continue;

    if (Keyword == "import" || Keyword == "abstract" || Line.find('$') != std::string::npos)
      return true;

    const bool IsBlock = Keyword == "material" || Keyword == "technique" || Keyword == "pass" || Keyword == "texture_unit";
    if (IsBlock && Line.find(':') != std::string::npos)
      return true;
  }

  return false;
}

} // namespace

//
// Constructor
//

CMaterialAssetType::CMaterialAssetType(std::shared_ptr<CMaterialEditingSession> _Session):
  m_Session(std::move(_Session))
{
  // Empty
}

//
// IAssetType
//

const TAssetTypeInfo & CMaterialAssetType::GetInfo() const noexcept
{
  static const TAssetTypeInfo Info{"material", "Material", {".material"}, true};
  return Info;
}

std::unique_ptr<CEditorWorkspace> CMaterialAssetType::OpenAsset(const TAssetRecord & _Asset, const std::string & _ResourceGroup, std::string & _Error) const
{
  std::ifstream File(_Asset.AbsolutePath, std::ios::binary);
  if (!File)
  {
    _Error = "Could not open material script: " + _Asset.AbsolutePath.string();
    return {};
  }

  const std::string Script((std::istreambuf_iterator<char>(File)), {});

  auto Materials = FindScriptMaterials(_Asset.RelativePath, _ResourceGroup);
  if (Materials.empty())
  {
    if (!ParseScript(_Asset.RelativePath, _ResourceGroup, _Error))
      return {};

    Materials = FindScriptMaterials(_Asset.RelativePath, _ResourceGroup);
  }

  if (Materials.empty())
  {
    _Error = "OGRE has no materials from this script. It may be empty or contain errors, see the OGRE log.";
    return {};
  }

  auto Instance = std::make_unique<CMaterialInstance>(_Asset, Materials, HasExpandedFeatures(Script));
  return std::make_unique<CMaterialWorkspace>(std::move(Instance), m_Session);
}

} // namespace editor
