#pragma once

#include <OgreAxisAlignedBox.h>
#include <OgrePrerequisites.h>
#include <OgreRay.h>
#include <OgreSharedPtr.h>

#include <cstdint>

namespace editor
{

class CRenderScene
{
public: // Constructor/Destructor

  CRenderScene();

  ~CRenderScene();

  CRenderScene(const CRenderScene &) = delete;

  CRenderScene & operator=(const CRenderScene &) = delete;

public: // Interface

  void Resize(unsigned int _Width, unsigned int _Height);

  void SetActive(bool _IsActive);

  const Ogre::TexturePtr & GetTexture() const noexcept;

  Ogre::SceneManager * GetSceneManager() const noexcept;

  Ogre::SceneNode * GetCameraNode() const noexcept;

  Ogre::SceneNode * GetContentRoot() const noexcept;

  Ogre::Ray GetViewportRay(float _NormalizedX, float _NormalizedY) const;

  Ogre::AxisAlignedBox GetContentBounds() const;

  std::uint64_t GetContentRevision() const noexcept;

  void MarkContentReplaced();

private: // Implementation

  void CreateRenderTarget(unsigned int _Width, unsigned int _Height);

  void DestroyRenderTarget();

private: // Attributes

  Ogre::SceneManager * m_SceneManager;
  Ogre::Camera *       m_Camera;
  Ogre::SceneNode *    m_CameraNode;
  Ogre::SceneNode *    m_ContentRoot;
  Ogre::TexturePtr     m_RenderTexture;
  unsigned int         m_Width;
  unsigned int         m_Height;
  bool                 m_IsActive;
  std::uint64_t        m_ContentRevision;
};

} // namespace editor
