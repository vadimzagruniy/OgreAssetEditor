#pragma once

#include "CameraPose.h"

#include <OgrePrerequisites.h>
#include <OgreQuaternion.h>

namespace editor
{

class CViewportCamera
{
public: // Constructor

  explicit CViewportCamera(Ogre::SceneNode * _CameraNode);

public: // Interface

  void Reset(const Ogre::AxisAlignedBox & _Bounds);

  void Frame(const Ogre::AxisAlignedBox & _Bounds);

  void AlignTo(const Ogre::Vector3 & _Direction);

  void Orbit(float _DeltaX, float _DeltaY);

  void Pan(float _DeltaX, float _DeltaY);

  void Dolly(float _Steps);

  void Move(const Ogre::Vector3 & _Direction, float _DeltaTime, bool _IsFast);

  void Update(float _DeltaTime);

  const TCameraPose & GetPose() const noexcept;

  Ogre::Quaternion GetOrientation() const;

  Ogre::Vector3 GetPosition() const;

private: // Implementation

  static Ogre::Quaternion CalcOrientation(const TCameraPose & _Pose);

  static Ogre::Vector3 CalcPosition(const TCameraPose & _Pose);

  void SetSceneRadius(float _Radius);

  void StartTransition(const TCameraPose & _Target);

  void ApplyPose();

private: // Attributes

  Ogre::SceneNode * m_CameraNode;
  TCameraPose       m_Pose;
  TCameraPose       m_TransitionStart;
  TCameraPose       m_TransitionTarget;
  float             m_TransitionTime;
  bool              m_IsTransitioning;
  float             m_SceneRadius;
};

} // namespace editor
