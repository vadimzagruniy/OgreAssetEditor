#pragma once

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

#include <memory>

namespace editor
{

enum class EPreviewShape;
class CRenderScene;

class CPreviewScene
{
public: // Constructor/Destructor

  CPreviewScene();

  ~CPreviewScene();

  CPreviewScene(const CPreviewScene &) = delete;

  CPreviewScene & operator=(const CPreviewScene &) = delete;

public: // Interface

  CRenderScene & GetRenderScene() const noexcept;

  void SetMesh(const Ogre::MeshPtr & _Mesh);

  void SetShape(EPreviewShape _Shape);

  void SetMaterial(const Ogre::MaterialPtr & _Material);

private: // Implementation

  void ShowEntity(Ogre::Entity * _Entity);

private: // Attributes

  std::unique_ptr<CRenderScene> m_RenderScene;
  Ogre::Entity *                m_Entity;
  Ogre::MaterialPtr             m_Material;
};

} // namespace editor
