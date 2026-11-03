#pragma once

#include "editor/viewport/gizmo/GizmoHandle.h"

#include <OgreQuaternion.h>
#include <OgreVector.h>

namespace editor
{

struct TGizmoDrag
{
  EGizmoHandle     Handle         = EGizmoHandle::None;
  Ogre::Vector3    Position       = Ogre::Vector3::ZERO;
  Ogre::Quaternion Orientation    = Ogre::Quaternion::IDENTITY;
  Ogre::Vector3    Scale          = Ogre::Vector3::UNIT_SCALE;
  Ogre::Vector3    Axis           = Ogre::Vector3::UNIT_X;
  Ogre::Vector3    StartPoint     = Ogre::Vector3::ZERO;
  float            StartParameter = 0.0f;
  float            StartMouseX    = 0.0f;
  float            PreviousAngle  = 0.0f;
  float            Angle          = 0.0f;
  float            AngleSign      = 1.0f;
  bool             IsCanceled     = false;
};

} // namespace editor
