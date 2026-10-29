#pragma once

#include <OgrePrerequisites.h>
#include <OgreVector.h>

namespace editor
{

class COrbitCamera
{
public: // Constructor

  explicit COrbitCamera(Ogre::SceneNode * _CameraNode);

public: // Interface

  void Focus(const Ogre::Vector3 & _Center, float _Radius);

  void Rotate(float _DeltaX, float _DeltaY);

  void Pan(float _DeltaX, float _DeltaY);

  void Zoom(float _WheelSteps);

private: // Implementation

  void UpdateNode();

private: // Attributes

  Ogre::SceneNode * m_CameraNode;
  Ogre::Vector3     m_Target;
  float             m_Yaw;
  float             m_Pitch;
  float             m_Distance;
  float             m_MinDistance;
  float             m_MaxDistance;
};

} // namespace editor
