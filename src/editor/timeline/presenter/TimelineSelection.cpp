#include "TimelineSelection.h"
#include "editor/timeline/model/Timeline.h"

namespace editor
{

//
// Constructor
//

CTimelineSelection::CTimelineSelection():
  m_Track(InvalidTimelineId)
{
  // Empty
}

//
// Interface
//

bool CTimelineSelection::IsEmpty() const noexcept
{
  return m_Items.empty() && m_Track == InvalidTimelineId;
}

bool CTimelineSelection::IsSelected(TTimelineId _ItemId) const
{
  return m_Items.contains(_ItemId);
}

const std::unordered_set<TTimelineId> & CTimelineSelection::GetItems() const noexcept
{
  return m_Items;
}

TTimelineId CTimelineSelection::GetTrack() const noexcept
{
  return m_Track;
}

void CTimelineSelection::SelectItem(TTimelineId _ItemId, bool _IsAdding)
{
  if (!_IsAdding)
    m_Items.clear();

  m_Items.insert(_ItemId);
}

void CTimelineSelection::ToggleItem(TTimelineId _ItemId)
{
  if (!m_Items.erase(_ItemId))
    m_Items.insert(_ItemId);
}

void CTimelineSelection::SelectTrack(TTimelineId _TrackId) noexcept
{
  m_Track = _TrackId;
}

void CTimelineSelection::ClearItems() noexcept
{
  m_Items.clear();
}

void CTimelineSelection::Clear() noexcept
{
  m_Items.clear();
  m_Track = InvalidTimelineId;
}

void CTimelineSelection::RemoveMissing(const CTimeline & _Timeline)
{
  std::erase_if(m_Items, [&_Timeline](TTimelineId _ItemId)
  {
    return !_Timeline.FindItemTrack(_ItemId);
  });

  if (m_Track != InvalidTimelineId && !_Timeline.HasTrack(m_Track))
    m_Track = InvalidTimelineId;
}

} // namespace editor
