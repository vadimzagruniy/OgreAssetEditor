#include "NodePropertyEvaluator.h"
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

CNodePropertyEvaluator::CNodePropertyEvaluator(std::weak_ptr<ISceneAnimationHost> _Host):
  m_Host(std::move(_Host))
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
  const auto Host = m_Host.lock();
  if (!Host || _Track.Target.Slot.empty())
    return;

  const auto Value = EvaluateKeys(_Track.Keys, _To);
  if (std::holds_alternative<std::monostate>(Value))
    return;

  Host->PreviewValue(_Track.Target.Slot, _Track.Target.Property, Value);
  m_TouchedNodes.insert(_Track.Target.Slot);
}

void CNodePropertyEvaluator::EndPreview()
{
  if (const auto Host = m_Host.lock())
  {
    for (const auto & NodeName : m_TouchedNodes)
      Host->RestoreNode(NodeName);
  }

  m_TouchedNodes.clear();
}

} // namespace editor
