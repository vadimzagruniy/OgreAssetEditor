#include "ViewportCamera.h"

#include <OgreAxisAlignedBox.h>
#include <OgreMath.h>
#include <OgreSceneNode.h>

#include <algorithm>
#include <cmath>

namespace editor
{

namespace
{

constexpr float RotationSpeed      = 0.006f;
constexpr float PanSpeed           = 0.0015f;
constexpr float DollyFactor        = 0.85f;
constexpr float MaxPitch           = Ogre::Math::HALF_PI;
constexpr float DefaultYaw         = 0.6f;
constexpr float DefaultPitch       = -0.35f;
constexpr float FitDistance        = 2.6f;
constexpr float MinRadius          = 0.01f;
constexpr float MinDistanceFactor  = 0.02f;
constexpr float MaxDistanceFactor  = 50.0f;
constexpr float MoveSpeedFactor    = 1.5f;
constexpr float FastMoveMultiplier = 4.0f;
constexpr float TransitionDuration = 0.25f;
constexpr float VerticalThreshold  = 0.999f;

float WrapAngle(float _Angle)
{
  return std::remainder(_Angle, Ogre::Math::TWO_PI);
}

float EaseOut(float _Progress)
{
  const float Remaining = 1.0f - _Progress;
  return 1.0f - Remaining * Remaining * Remaining;
}

float CalcRadius(const Ogre::AxisAlignedBox & _Bounds)
{
  if (!_Bounds.isFinite())
    return 1.0f;

  return std::max(_Bounds.getHalfSize().length(), MinRadius);
}

Ogre::Vector3 CalcCenter(const Ogre::AxisAlignedBox & _Bounds)
{
  return _Bounds.isFinite() ? _Bounds.getCenter() : Ogre::Vector3::ZERO;
}

} // namespace

//
// Constructor
//

CViewportCamera::CViewportCamera(Ogre::SceneNode * _CameraNode):
  m_CameraNode(_CameraNode),
  m_Pose{Ogre::Vector3::ZERO, DefaultYaw, DefaultPitch, 10.0f},
  m_TransitionTime(0.0f),
  m_IsTransitioning(false),
  m_SceneRadius(1.0f)
{
  ApplyPose();
}

//
// Interface
//

void CViewportCamera::Reset(const Ogre::AxisAlignedBox & _Bounds)
{
  SetSceneRadius(CalcRadius(_Bounds));

  m_Pose            = {CalcCenter(_Bounds), DefaultYaw, DefaultPitch, m_SceneRadius * FitDistance};
  m_IsTransitioning = false;

  ApplyPose();
}

void CViewportCamera::Frame(const Ogre::AxisAlignedBox & _Bounds)
{
  const float Radius = CalcRadius(_Bounds);

  TCameraPose Target = m_Pose;
  Target.Pivot    = CalcCenter(_Bounds);
  Target.Distance = Radius * FitDistance;

  StartTransition(Target);
}

void CViewportCamera::SetSceneBounds(const Ogre::AxisAlignedBox & _Bounds)
{
  SetSceneRadius(CalcRadius(_Bounds));
}

void CViewportCamera::AlignTo(const Ogre::Vector3 & _Direction)
{
  const Ogre::Vector3 Direction = _Direction.normalisedCopy();

  TCameraPose Target = m_Pose;
  Target.Pitch = -std::asin(std::clamp(Direction.y, -1.0f, 1.0f));

  // Looking straight up or down leaves the yaw undefined, so the current one is kept.
  if (std::abs(Direction.y) < VerticalThreshold)
    Target.Yaw = std::atan2(Direction.x, Direction.z);

  StartTransition(Target);
}

void CViewportCamera::Orbit(float _DeltaX, float _DeltaY)
{
  m_IsTransitioning = false;

  m_Pose.Yaw  -= _DeltaX * RotationSpeed;
  m_Pose.Pitch = std::clamp(m_Pose.Pitch - _DeltaY * RotationSpeed, -MaxPitch, MaxPitch);

  ApplyPose();
}

void CViewportCamera::Pan(float _DeltaX, float _DeltaY)
{
  m_IsTransitioning = false;

  const Ogre::Quaternion Orientation = CalcOrientation(m_Pose);
  const float            Scale       = m_Pose.Distance * PanSpeed;

  m_Pose.Pivot += Orientation.xAxis() * (-_DeltaX * Scale) + Orientation.yAxis() * (_DeltaY * Scale);

  ApplyPose();
}

void CViewportCamera::Dolly(float _Steps)
{
  m_IsTransitioning = false;

  const float MinDistance = m_SceneRadius * MinDistanceFactor;
  const float MaxDistance = m_SceneRadius * MaxDistanceFactor;

  m_Pose.Distance = std::clamp(m_Pose.Distance * std::pow(DollyFactor, _Steps), MinDistance, MaxDistance);

  ApplyPose();
}

void CViewportCamera::Move(const Ogre::Vector3 & _Direction, float _DeltaTime, bool _IsFast)
{
  if (_Direction.isZeroLength())
    return;

  m_IsTransitioning = false;

  // The pivot moves together with the camera, so orbiting continues around the point in front of it.
  const float Speed = m_SceneRadius * MoveSpeedFactor * (_IsFast ? FastMoveMultiplier : 1.0f);

  m_Pose.Pivot += CalcOrientation(m_Pose) * _Direction.normalisedCopy() * (Speed * _DeltaTime);

  ApplyPose();
}

void CViewportCamera::Update(float _DeltaTime)
{
  if (!m_IsTransitioning)
    return;

  m_TransitionTime += _DeltaTime;

  const float Progress = std::min(m_TransitionTime / TransitionDuration, 1.0f);
  const float Blend    = EaseOut(Progress);

  m_Pose.Pivot    = m_TransitionStart.Pivot + (m_TransitionTarget.Pivot - m_TransitionStart.Pivot) * Blend;
  m_Pose.Yaw      = m_TransitionStart.Yaw + WrapAngle(m_TransitionTarget.Yaw - m_TransitionStart.Yaw) * Blend;
  m_Pose.Pitch    = m_TransitionStart.Pitch + (m_TransitionTarget.Pitch - m_TransitionStart.Pitch) * Blend;
  m_Pose.Distance = m_TransitionStart.Distance * std::pow(m_TransitionTarget.Distance / m_TransitionStart.Distance, Blend);

  if (Progress >= 1.0f)
    m_IsTransitioning = false;

  ApplyPose();
}

const TCameraPose & CViewportCamera::GetPose() const noexcept
{
  return m_Pose;
}

Ogre::Quaternion CViewportCamera::GetOrientation() const
{
  return CalcOrientation(m_Pose);
}

Ogre::Vector3 CViewportCamera::GetPosition() const
{
  return CalcPosition(m_Pose);
}

//
// Implementation
//

Ogre::Quaternion CViewportCamera::CalcOrientation(const TCameraPose & _Pose)
{
  const Ogre::Quaternion YawRotation(Ogre::Radian(_Pose.Yaw), Ogre::Vector3::UNIT_Y);
  const Ogre::Quaternion PitchRotation(Ogre::Radian(_Pose.Pitch), Ogre::Vector3::UNIT_X);

  return YawRotation * PitchRotation;
}

Ogre::Vector3 CViewportCamera::CalcPosition(const TCameraPose & _Pose)
{
  return _Pose.Pivot + CalcOrientation(_Pose) * Ogre::Vector3(0.0f, 0.0f, _Pose.Distance);
}

void CViewportCamera::SetSceneRadius(float _Radius)
{
  m_SceneRadius = std::max(_Radius, MinRadius);
}

void CViewportCamera::StartTransition(const TCameraPose & _Target)
{
  m_TransitionStart  = m_Pose;
  m_TransitionTarget = _Target;
  m_TransitionTime   = 0.0f;
  m_IsTransitioning  = true;
}

void CViewportCamera::ApplyPose()
{
  m_CameraNode->setOrientation(CalcOrientation(m_Pose));
  m_CameraNode->setPosition(CalcPosition(m_Pose));
}

} // namespace editor
