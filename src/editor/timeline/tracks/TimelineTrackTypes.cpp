#include "TimelineTrackTypes.h"
#include "ITimelineTrackEvaluator.h"
#include "editor/timeline/model/TimelineTrack.h"

#include <cassert>
#include <utility>

namespace editor
{

//
// Constructor
//

CTimelineTrackTypes::CTimelineTrackTypes()
{
  Register({std::string(TimelineGroupType), "Group", ETimelineItemKind::Group}, nullptr);
}

//
// Interface
//

void CTimelineTrackTypes::Register(TTimelineTrackTypeInfo _Info, std::shared_ptr<ITimelineTrackEvaluator> _Evaluator)
{
  if (_Evaluator)
    m_Evaluators.push_back(_Evaluator);

  auto TypeId = _Info.TypeId;

  const auto [TypeIterator, IsInserted] = m_Types.try_emplace(std::move(TypeId), TEntry{std::move(_Info), std::move(_Evaluator)});
  assert(IsInserted && "Timeline track type is already registered");
  (void)TypeIterator;
  (void)IsInserted;
}

ETimelineItemKind CTimelineTrackTypes::GetItemKind(const std::string & _TypeId) const
{
  const auto TypeIterator = m_Types.find(_TypeId);
  return TypeIterator == m_Types.end() ? ETimelineItemKind::Keys : TypeIterator->second.Info.ItemKind;
}

std::shared_ptr<ITimelineTrackEvaluator> CTimelineTrackTypes::FindEvaluator(const std::string & _TypeId) const
{
  const auto TypeIterator = m_Types.find(_TypeId);
  return TypeIterator == m_Types.end() ? nullptr : TypeIterator->second.Evaluator;
}

const std::vector<std::shared_ptr<ITimelineTrackEvaluator>> & CTimelineTrackTypes::GetEvaluators() const noexcept
{
  return m_Evaluators;
}

} // namespace editor
