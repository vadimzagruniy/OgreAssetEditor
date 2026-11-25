#include "AnimationSlots.h"
#include "editor/assets/AssetRecord.h"
#include "editor/documents/AssetDocument.h"
#include "modules/scene/animation/ISceneAnimationHost.h"

#include <utility>

namespace editor
{

//
// Constructor
//

CAnimationSlots::CAnimationSlots(std::weak_ptr<ISceneAnimationHost> _Host, std::shared_ptr<const CAssetDocument> _Animation):
  m_Host(std::move(_Host)),
  m_Animation(std::move(_Animation))
{
  // Empty
}

//
// Interface
//

std::shared_ptr<ISceneAnimationHost> CAnimationSlots::LockHost() const
{
  return m_Host.lock();
}

std::string CAnimationSlots::GetAnimationPath() const
{
  return m_Animation->GetRecord().RelativePath.generic_string();
}

std::string CAnimationSlots::ResolveSlot(const std::string & _Slot) const
{
  const auto Host = LockHost();
  return Host ? Host->ResolveSlot(GetAnimationPath(), _Slot) : _Slot;
}

} // namespace editor
