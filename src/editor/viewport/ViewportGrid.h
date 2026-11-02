#pragma once

#include <OgrePrerequisites.h>

namespace editor
{

class CViewportGrid
{
public: // Constructor/Destructor

  explicit CViewportGrid(Ogre::SceneManager * _SceneManager);

  ~CViewportGrid();

  CViewportGrid(const CViewportGrid &) = delete;

  CViewportGrid & operator=(const CViewportGrid &) = delete;

public: // Interface

  bool IsAvailable() const noexcept;

  bool IsVisible() const noexcept;

  void SetVisible(bool _IsVisible);

  void Update(const Ogre::Vector3 & _CameraPosition, float _ViewDistance);

private: // Implementation

  void CreatePlane();

private: // Attributes

  Ogre::SceneManager * m_SceneManager;
  Ogre::SceneNode *    m_Node;
  Ogre::ManualObject * m_Plane;
  bool                 m_IsVisible;
};

} // namespace editor
