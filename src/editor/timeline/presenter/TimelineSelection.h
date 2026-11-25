#pragma once

#include "editor/timeline/model/TimelineId.h"

#include <unordered_set>

namespace editor
{

class CTimeline;

class CTimelineSelection
{
public: // Constructor

  CTimelineSelection();

public: // Interface

  bool IsEmpty() const noexcept;

  bool IsSelected(TTimelineId _ItemId) const;

  const std::unordered_set<TTimelineId> & GetItems() const noexcept;

  TTimelineId GetTrack() const noexcept;

  void SelectItem(TTimelineId _ItemId, bool _IsAdding);

  void ToggleItem(TTimelineId _ItemId);

  void SelectTrack(TTimelineId _TrackId) noexcept;

  void ClearItems() noexcept;

  void Clear() noexcept;

  void RemoveMissing(const CTimeline & _Timeline);

private: // Attributes

  std::unordered_set<TTimelineId> m_Items;
  TTimelineId                     m_Track;
};

} // namespace editor
