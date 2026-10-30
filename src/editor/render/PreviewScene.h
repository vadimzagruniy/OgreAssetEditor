#pragma once

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

#include <memory>

namespace editor
{

enum class EPreviewShape;
class COrbitCamera;

class CPreviewScene
{
public: // Constructor/Destructor

  CPreviewScene();

  ~CPreviewScene();

  CPreviewScene(const CPreviewScene &) = delete;

  CPreviewScene & operator=(const CPreviewScene &) = delete;

public: // Interface

  void SetMesh(const Ogre::MeshPtr & _Mesh);

  void SetShape(EPreviewShape _Shape);

  void SetMaterial(const Ogre::MaterialPtr & _Material);

  void Resize(unsigned int _Width, unsigned int _Height);

  void SetActive(bool _IsActive);

  const Ogre::TexturePtr & GetTexture() const noexcept;

  COrbitCamera & GetCamera() noexcept;

private: // Implementation

  void ShowEntity(Ogre::Entity * _Entity);

  void CreateRenderTarget(unsigned int _Width, unsigned int _Height);

  void DestroyRenderTarget();

private: // Attributes

  Ogre::SceneManager *          m_SceneManager;
  Ogre::Camera *                m_Camera;
  Ogre::SceneNode *             m_MeshNode;
  Ogre::Entity *                m_Entity;
  Ogre::MaterialPtr             m_Material;
  std::unique_ptr<COrbitCamera> m_OrbitCamera;
  Ogre::TexturePtr              m_RenderTexture;
  unsigned int                  m_Width;
  unsigned int                  m_Height;
  bool                          m_IsActive;
};

} // namespace editor
