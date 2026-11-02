#pragma once

#include <OgreQuaternion.h>
#include <OgreVector.h>

#include <optional>

struct ImVec2;

namespace editor
{

std::optional<Ogre::Vector3> DrawViewAxisGizmo(const ImVec2 & _ViewportOrigin, const ImVec2 & _ViewportSize, const Ogre::Quaternion & _CameraOrientation);

} // namespace editor
