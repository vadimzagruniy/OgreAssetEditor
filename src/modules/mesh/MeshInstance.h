#pragma once

#include "editor/workspaces/AssetInstance.h"

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

namespace editor
{

class CMeshInstance final:
  public CAssetInstance
{
public: // Constructor/Destructor

  CMeshInstance(TAssetRecord _Record, Ogre::MeshPtr _Mesh);

  ~CMeshInstance() override;

public: // Interface

  const Ogre::MeshPtr & GetMesh() const noexcept;

public: // CAssetInstance

  std::vector<TAssetProperty> GetProperties() const override;

private: // Attributes

  Ogre::MeshPtr m_Mesh;
};

} // namespace editor
