#pragma once

#include "TimelineTrackTemplate.h"
#include "editor/timeline/model/TimelineId.h"
#include "editor/timeline/model/TimelineTicks.h"

#include <string>
#include <vector>

namespace editor
{

class CTimeline;
class CTimelineSelection;
enum class ETimelineItemKind;
enum class ETimelineSelectMode;

class ITimelinePresenter
{
public: // Destructor

  virtual ~ITimelinePresenter();

public: // State

  virtual const CTimeline & GetTimeline() const = 0;

  virtual const CTimelineSelection & GetSelection() const = 0;

  virtual TTimelineTicks GetTime() const = 0;

  virtual bool IsPlaying() const = 0;

  virtual bool IsPreviewing() const = 0;

  virtual bool IsLooping() const = 0;

  virtual ETimelineItemKind GetItemKind(const TTimelineTrack & _Track) const = 0;

  virtual bool CanAddKey(TTimelineId _TrackId) const = 0;

  virtual bool IsTargetValid(const TTimelineTrack & _Track) const = 0;

  virtual bool CanBindToSelection(const TTimelineTrack & _Track) const = 0;

  virtual std::vector<TTimelineTrackTemplate> GetTrackTemplates() const = 0;

public: // Actions

  virtual void Play() = 0;

  virtual void Pause() = 0;

  virtual void Stop() = 0;

  virtual void SetLooping(bool _IsLooping) = 0;

  virtual void Scrub(TTimelineTicks _Time) = 0;

  virtual void SelectItem(TTimelineId _ItemId, ETimelineSelectMode _Mode) = 0;

  virtual void SelectItems(const std::vector<TTimelineId> & _ItemIds, bool _IsAdding) = 0;

  virtual void SelectTrack(TTimelineId _TrackId) = 0;

  virtual void PreviewMoveSelection(TTimelineTicks _Delta) = 0;

  virtual void CommitMove() = 0;

  virtual void DeleteSelection() = 0;

  virtual void AddKey(TTimelineId _TrackId, TTimelineTicks _Time) = 0;

  virtual void AddTrack(const TTimelineTrack & _Track) = 0;

  virtual void DeleteTrack(TTimelineId _TrackId) = 0;

  virtual void RenameTrack(TTimelineId _TrackId, const std::string & _Name) = 0;

  virtual void BindToSelection(const TTimelineTrack & _Track) = 0;
};

} // namespace editor
