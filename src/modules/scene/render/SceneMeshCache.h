#pragma once

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

#include <string>
#include <unordered_map>

namespace editor
{

class CSceneMeshCache final
{
public: // Constructor/Destructor

  explicit CSceneMeshCache(std::string _ResourceGroup);

  ~CSceneMeshCache();

  CSceneMeshCache(const CSceneMeshCache &) = delete;

  CSceneMeshCache & operator=(const CSceneMeshCache &) = delete;

public: // Interface

  bool Acquire(const std::string & _MeshName, Ogre::MeshPtr & _Mesh, std::string & _Error);

private: // Attributes

  std::string                                    m_ResourceGroup;
  std::unordered_map<std::string, Ogre::MeshPtr> m_Meshes;
};

} // namespace editor
