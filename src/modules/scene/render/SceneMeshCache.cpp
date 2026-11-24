#include "SceneMeshCache.h"
#include "editor/assets/ResourceRelease.h"

#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreResourceGroupManager.h>

#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CSceneMeshCache::CSceneMeshCache(std::string _ResourceGroup):
  m_ResourceGroup(std::move(_ResourceGroup))
{
  // Empty
}

CSceneMeshCache::~CSceneMeshCache()
{
  for (auto & Entry : m_Meshes)
    ReleaseOgreResource(std::move(Entry.second));
}

//
// Interface
//

bool CSceneMeshCache::Acquire(const std::string & _MeshName, Ogre::MeshPtr & _Mesh, std::string & _Error)
{
  if (const auto MeshIterator = m_Meshes.find(_MeshName); MeshIterator != m_Meshes.end())
  {
    _Mesh = MeshIterator->second;
    return true;
  }

  if (!Ogre::ResourceGroupManager::getSingleton().resourceExists(m_ResourceGroup, _MeshName))
  {
    _Error = "Mesh '" + _MeshName + "' is not found in the project resources.";
    return false;
  }

  _Mesh = Ogre::MeshManager::getSingleton().load(_MeshName, m_ResourceGroup);
  if (!_Mesh)
  {
    _Error = "OGRE returned an empty mesh: " + _MeshName;
    return false;
  }

  m_Meshes.emplace(_MeshName, _Mesh);

  return true;
}

} // namespace editor
