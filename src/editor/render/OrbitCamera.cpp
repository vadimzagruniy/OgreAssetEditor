#include "OrbitCamera.h"

#include <OgreQuaternion.h>
#include <OgreSceneNode.h>

#include <algorithm>
#include <cmath>

namespace editor
{

namespace
{

constexpr float RotationSpeed  = 0.01f;
constexpr float PanSpeed       = 0.0015f;
constexpr float ZoomFactor     = 0.85f;
constexpr float MaxPitch       = 1.5f;
constexpr float DefaultYaw     = 0.6f;
constexpr float DefaultPitch   = -0.35f;
constexpr float FitDistance    = 2.6f;
constexpr float MinRadius      = 0.01f;

} // namespace

//
// Constructor
//

COrbitCamera::COrbitCamera(Ogre::SceneNode * _CameraNode):
  m_CameraNode(_CameraNode),
  m_Target(Ogre::Vector3::ZERO),
  m_Yaw(DefaultYaw),
  m_Pitch(DefaultPitch),
  m_Distance(10.0f),
  m_MinDistance(0.1f),
  m_MaxDistance(1000.0f)
{
  UpdateNode();
}

//
// Interface
//

void COrbitCamera::Focus(const Ogre::Vector3 & _Center, float _Radius)
{
  const float Radius = std::max(_Radius, MinRadius);

  m_Target      = _Center;
  m_Yaw         = DefaultYaw;
  m_Pitch       = DefaultPitch;
  m_Distance    = Radius * FitDistance;
  m_MinDistance = Radius * 0.1f;
  m_MaxDistance = Radius * 50.0f;

  UpdateNode();
}

void COrbitCamera::Rotate(float _DeltaX, float _DeltaY)
{
  m_Yaw  -= _DeltaX * RotationSpeed;
  m_Pitch = std::clamp(m_Pitch - _DeltaY * RotationSpeed, -MaxPitch, MaxPitch);

  UpdateNode();
}

void COrbitCamera::Pan(float _DeltaX, float _DeltaY)
{
  const Ogre::Quaternion & Orientation = m_CameraNode->getOrientation();
  const float              Scale       = m_Distance * PanSpeed;

  m_Target += Orientation.xAxis() * (-_DeltaX * Scale) + Orientation.yAxis() * (_DeltaY * Scale);

  UpdateNode();
}

void COrbitCamera::Zoom(float _WheelSteps)
{
  m_Distance = std::clamp(m_Distance * std::pow(ZoomFactor, _WheelSteps), m_MinDistance, m_MaxDistance);

  UpdateNode();
}

//
// Implementation
//

void COrbitCamera::UpdateNode()
{
  const Ogre::Quaternion YawRotation(Ogre::Radian(m_Yaw), Ogre::Vector3::UNIT_Y);
  const Ogre::Quaternion PitchRotation(Ogre::Radian(m_Pitch), Ogre::Vector3::UNIT_X);
  const Ogre::Quaternion Orientation = YawRotation * PitchRotation;

  m_CameraNode->setOrientation(Orientation);
  m_CameraNode->setPosition(m_Target + Orientation * Ogre::Vector3(0.0f, 0.0f, m_Distance));
}

} // namespace editor
