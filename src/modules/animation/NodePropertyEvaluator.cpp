#include "NodePropertyEvaluator.h"
#include "AnimationSlots.h"
#include "editor/timeline/model/TimelineTrack.h"
#include "editor/timeline/playback/TimelineKeyEvaluation.h"
#include "modules/scene/animation/ISceneAnimationHost.h"

#include <utility>
#include <variant>

namespace editor
{

//
// Constructor
//

CNodePropertyEvaluator::CNodePropertyEvaluator(std::shared_ptr<const CAnimationSlots> _Slots):
  m_Slots(std::move(_Slots))
{
  // Empty
}

//
// ITimelineTrackEvaluator
//

void CNodePropertyEvaluator::BeginPreview()
{
  m_TouchedNodes.clear();
}

void CNodePropertyEvaluator::Evaluate(const TTimelineTrack & _Track, TTimelineTicks /*_From*/, TTimelineTicks _To, ETimelineEvaluateMode /*_Mode*/)
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Track.Target.Slot.empty())
    return;

  const auto Value = EvaluateKeys(_Track.Keys, _To);
  if (std::holds_alternative<std::monostate>(Value))
    return;

  const auto NodeName = m_Slots->ResolveSlot(_Track.Target.Slot);

  Host->PreviewValue(NodeName, _Track.Target.Property, Value);
  m_TouchedNodes.insert(NodeName);
}

void CNodePropertyEvaluator::EndPreview()
{
  if (const auto Host = m_Slots->LockHost())
  {
    for (const auto & NodeName : m_TouchedNodes)
      Host->RestoreNode(NodeName);
  }

  m_TouchedNodes.clear();
}

} // namespace editor
