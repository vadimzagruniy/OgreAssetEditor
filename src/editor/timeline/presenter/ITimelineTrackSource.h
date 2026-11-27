#pragma once

#include "TimelineTrackTemplate.h"
#include "editor/inspector/PropertyValue.h"

#include <cstdint>
#include <string>
#include <vector>

namespace editor
{

class CTimeline;

class ITimelineTrackSource
{
public: // Destructor

  virtual ~ITimelineTrackSource();

public: // Interface

  virtual std::vector<TTimelineTrackTemplate> GetTrackTemplates(const CTimeline & _Timeline) const = 0;

  virtual TPropertyValue CaptureValue(const TTimelineTrack & _Track) const = 0;

  virtual bool IsTargetValid(const TTimelineTrack & _Track) const = 0;

  virtual bool CanBindToSelection(const TTimelineTrack & _Track) const = 0;

  virtual void BindToSelection(const TTimelineTrack & _Track) const = 0;

  virtual std::uint32_t GetTrackColor(const TTimelineTrack & _Track) const = 0;

  virtual bool HasTrackTarget() const = 0;

  virtual void SelectTrackTarget(const std::string & _Target) const = 0;

public: // Events

  virtual void OnTrackAdded(const TTimelineTrack & _Track) const = 0;
};

} // namespace editor
