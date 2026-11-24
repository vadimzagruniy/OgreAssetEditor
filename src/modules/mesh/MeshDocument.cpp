#include "MeshDocument.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/ResourceRelease.h"
#include "editor/documents/AssetProperty.h"

#include <OgreMesh.h>

#include <string>
#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CMeshDocument::CMeshDocument(TAssetRecord _Record, Ogre::MeshPtr _Mesh):
  CAssetDocument(std::move(_Record), "Mesh"),
  m_Mesh(std::move(_Mesh))
{
  // Empty
}

CMeshDocument::~CMeshDocument()
{
  ReleaseOgreResource(std::move(m_Mesh));
}

//
// CAssetDocument
//

std::vector<TAssetProperty> CMeshDocument::GetProperties() const
{
  return {{"Submeshes",   std::to_string(m_Mesh->getNumSubMeshes())          },
          {"Vertex data", m_Mesh->sharedVertexData ? "Shared" : "Per submesh"},
          {"Animations",  std::to_string(m_Mesh->getNumAnimations())         }};
}

//
// Interface
//

const Ogre::MeshPtr & CMeshDocument::GetMesh() const noexcept
{
  return m_Mesh;
}

} // namespace editor
