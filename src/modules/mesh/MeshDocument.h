#pragma once

#include "editor/documents/AssetDocument.h"

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

namespace editor
{

class CMeshDocument final:
  public CAssetDocument
{
public: // Constructor/Destructor

  CMeshDocument(TAssetRecord _Record, Ogre::MeshPtr _Mesh);

  ~CMeshDocument() override;

public: // CAssetDocument

  std::vector<TAssetProperty> GetProperties() const override;

public: // Interface

  const Ogre::MeshPtr & GetMesh() const noexcept;

private: // Attributes

  Ogre::MeshPtr m_Mesh;
};

} // namespace editor
