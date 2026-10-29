#include "ResourceRelease.h"

#include <OgreResource.h>
#include <OgreResourceGroupManager.h>
#include <OgreResourceManager.h>

namespace editor
{

void ReleaseOgreResource(Ogre::ResourcePtr _Resource)
{
  if (!_Resource)
    return;

  // References held by the resource system plus the one passed into this function.
  const long OwnedReferences = Ogre::ResourceGroupManager::RESOURCE_SYSTEM_NUM_REFERENCE_COUNTS + 1;

  // Somebody else (e.g. a material or a scene entity) still uses the resource.
  if (_Resource.use_count() > OwnedReferences)
    return;

  if (auto * Creator = _Resource->getCreator())
    Creator->remove(_Resource);
}

} // namespace editor
