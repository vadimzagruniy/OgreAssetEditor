#pragma once

#include "ITimelinePresenter.h"
#include "editor/inspector/PropertyValue.h"
#include "editor/timeline/model/TimelineTrackLocation.h"

#include <cstddef>
#include <memory>

namespace editor
{

namespace core
{
class CActionQueue;
} // namespace core

class CTimelineDocument;
class CTimelineKeyPropertySource;
class IPropertySource;
class CTimelinePlayer;
class CTimelineTrackTypes;
class ITimelineTrackSource;
struct TTimelineChange;

class CTimelinePresenter final:
  public ITimelinePresenter
{
public: // Constructor/Destructor

  CTimelinePresenter(std::shared_ptr<CTimelineDocument> _Document, std::shared_ptr<const CTimelineTrackTypes> _TrackTypes, std::shared_ptr<const ITimelineTrackSource> _TrackSource, std::shared_ptr<core::CActionQueue> _ActionQueue);

  ~CTimelinePresenter() override;

  CTimelinePresenter(const CTimelinePresenter &) = delete;

  CTimelinePresenter & operator=(const CTimelinePresenter &) = delete;

public: // ITimelinePresenter

  const CTimeline & GetTimeline() const override;

  const CTimelineSelection & GetSelection() const override;

  TTimelineTicks GetTime() const override;

  bool IsPlaying() const override;

  bool IsPreviewing() const override;

  bool IsLooping() const override;

  ETimelineItemKind GetItemKind(const TTimelineTrack & _Track) const override;

  bool CanAddKey(TTimelineId _TrackId) const override;

  bool IsTargetValid(const TTimelineTrack & _Track) const override;

  bool CanBindToSelection(const TTimelineTrack & _Track) const override;

  std::vector<TTimelineTrackTemplate> GetTrackTemplates() const override;

  void Play() override;

  void Pause() override;

  void Stop() override;

  void SetLooping(bool _IsLooping) override;

  void Scrub(TTimelineTicks _Time) override;

  void SelectItem(TTimelineId _ItemId, ETimelineSelectMode _Mode) override;

  void SelectItems(const std::vector<TTimelineId> & _ItemIds, bool _IsAdding) override;

  void SelectTrack(TTimelineId _TrackId) override;

  void PreviewMoveSelection(TTimelineTicks _Delta) override;

  void CommitMove() override;

  void DeleteSelection() override;

  void AddKey(TTimelineId _TrackId, TTimelineTicks _Time) override;

  void AddTrack(const TTimelineTrack & _Track) override;

  void DeleteTrack(TTimelineId _TrackId) override;

  void RenameTrack(TTimelineId _TrackId, const std::string & _Name) override;

  void BindToSelection(const TTimelineTrack & _Track) override;

public: // Interface

  void Tick(float _DeltaTime);

  std::shared_ptr<IPropertySource> GetInspected() const;

private: // Implementation

  std::vector<TTimelineId> GetSelectedItems() const;

  TPropertyValue FindKeyValue(const TTimelineTrack & _Track, TTimelineTicks _Time) const;

  TTimelineTrackLocation FindNewTrackLocation() const;

private: // Events

  void OnDocumentChanged(const TTimelineChange & _Change);

private: // Attributes

  std::shared_ptr<CTimelineDocument>          m_Document;
  std::shared_ptr<const CTimelineTrackTypes>  m_TrackTypes;
  std::unique_ptr<CTimelinePlayer>            m_Player;
  std::shared_ptr<const ITimelineTrackSource> m_TrackSource;
  std::shared_ptr<CTimelineSelection>         m_Selection;
  std::shared_ptr<CTimelineKeyPropertySource> m_KeyProperties;
  std::shared_ptr<core::CActionQueue>         m_ActionQueue;
  std::size_t                                 m_ChangedConnection;
};

} // namespace editor
