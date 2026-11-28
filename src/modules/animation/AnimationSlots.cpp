#include "AnimationSlots.h"
#include "editor/assets/AssetRecord.h"
#include "editor/assets/AssetReference.h"
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

const TAssetId & CAnimationSlots::GetAnimationId() const
{
  return m_Animation->GetRecord().Id;
}

TAssetReference CAnimationSlots::MakeAnimationRef() const
{
  const auto & Record = m_Animation->GetRecord();

  return TAssetReference{Record.Id, Record.RelativePath.generic_string()};
}

std::string CAnimationSlots::ResolveSlot(const std::string & _Slot) const
{
  const auto Host = LockHost();
  return Host ? Host->ResolveSlot(GetAnimationId(), _Slot) : _Slot;
}

} // namespace editor
