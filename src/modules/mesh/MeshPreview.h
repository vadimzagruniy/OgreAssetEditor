#pragma once

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

#include <memory>

namespace editor
{

class COrbitCamera;

class CMeshPreview
{
public: // Constructor/Destructor

  explicit CMeshPreview(const Ogre::MeshPtr & _Mesh);

  ~CMeshPreview();

  CMeshPreview(const CMeshPreview &) = delete;

  CMeshPreview & operator=(const CMeshPreview &) = delete;

public: // Interface

  void Resize(unsigned int _Width, unsigned int _Height);

  void SetActive(bool _IsActive);

  const Ogre::TexturePtr & GetTexture() const noexcept;

  COrbitCamera & GetCamera() noexcept;

private: // Implementation

  void CreateRenderTarget(unsigned int _Width, unsigned int _Height);

  void DestroyRenderTarget();

private: // Attributes

  Ogre::SceneManager *          m_SceneManager;
  Ogre::Camera *                m_Camera;
  std::unique_ptr<COrbitCamera> m_OrbitCamera;
  Ogre::TexturePtr              m_RenderTexture;
  unsigned int                  m_Width;
  unsigned int                  m_Height;
  bool                          m_IsActive;
};

} // namespace editor
