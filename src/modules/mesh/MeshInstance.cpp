#include "MeshInstance.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/ResourceRelease.h"
#include "editor/workspaces/AssetProperty.h"

#include <OgreMesh.h>

#include <string>
#include <utility>

namespace editor
{

//
// Constructor/Destructor
//

CMeshInstance::CMeshInstance(TAssetRecord _Record, Ogre::MeshPtr _Mesh):
  CAssetInstance(std::move(_Record), "Mesh"),
  m_Mesh(std::move(_Mesh))
{
  // Empty
}

CMeshInstance::~CMeshInstance()
{
  ReleaseOgreResource(std::move(m_Mesh));
}

//
// Interface
//

const Ogre::MeshPtr & CMeshInstance::GetMesh() const noexcept
{
  return m_Mesh;
}

//
// CAssetInstance
//

std::vector<TAssetProperty> CMeshInstance::GetProperties() const
{
  return {{"Submeshes",   std::to_string(m_Mesh->getNumSubMeshes())          },
          {"Vertex data", m_Mesh->sharedVertexData ? "Shared" : "Per submesh"},
          {"Animations",  std::to_string(m_Mesh->getNumAnimations())         }};
}

} // namespace editor
