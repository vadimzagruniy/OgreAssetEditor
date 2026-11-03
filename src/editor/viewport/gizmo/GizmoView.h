#pragma once

#include <OgreMatrix4.h>
#include <OgreQuaternion.h>
#include <OgreVector.h>
#include <imgui.h>

#include <array>

namespace editor
{

struct TGizmoView
{
  ImVec2                       Origin;
  ImVec2                       Size;
  Ogre::Matrix4                Projection;
  Ogre::Affine3                View;
  Ogre::Vector3                ToCamera;
  Ogre::Vector3                Position;
  Ogre::Quaternion             Orientation;
  Ogre::Vector3                Scale;
  std::array<Ogre::Vector3, 3> Axes;
  ImVec2                       ScreenPosition;
  float                        Length      = 0.0f;
  float                        PixelLength = 0.0f;
};

} // namespace editor
