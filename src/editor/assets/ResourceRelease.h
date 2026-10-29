#pragma once

#include <OgrePrerequisites.h>
#include <OgreSharedPtr.h>

namespace editor
{

// Removes the resource from its OGRE manager if nobody outside the resource system uses it.
// Takes over the caller's reference, so pass the owning pointer with std::move.
void ReleaseOgreResource(Ogre::ResourcePtr _Resource);

} // namespace editor
