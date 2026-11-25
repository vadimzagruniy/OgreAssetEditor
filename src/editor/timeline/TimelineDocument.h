#pragma once

#include "core/Signal.h"
#include "editor/documents/AssetDocument.h"
#include "editor/inspector/PropertyValue.h"
#include "editor/timeline/model/Timeline.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace editor
{

enum class ETimelineChangeKind;
struct TTimelineChange;

class CTimelineDocument:
  public CAssetDocument,
  public std::enable_shared_from_this<CTimelineDocument>
{
public: // Types

  using TChangedSignal = core::CSignal<const TTimelineChange &>;

public: // Constructor/Destructor

  CTimelineDocument(TAssetRecord _Record, std::string _TypeName);

  ~CTimelineDocument() override;

public: // Interface

  const CTimeline & GetTimeline() const noexcept;

  void SetDuration(TTimelineTicks _Duration);

  void SetFrameRate(int _FrameRate);

  TTimelineId AddTrack(const TTimelineTrackLocation & _Location, TTimelineTrack _Track);

  void RemoveTrack(TTimelineId _TrackId);

  void MoveTrack(TTimelineId _TrackId, const TTimelineTrackLocation & _Location);

  void RenameTrack(TTimelineId _TrackId, const std::string & _Name);

  void SetTrackTarget(TTimelineId _TrackId, const TTimelineTarget & _Target);

  TTimelineId AddKey(TTimelineId _TrackId, TTimelineKey _Key);

  TTimelineId AddClip(TTimelineId _TrackId, TTimelineClip _Clip);

  void RemoveItems(const std::vector<TTimelineId> & _ItemIds);

  void SetKeyValue(TTimelineId _KeyId, const TPropertyValue & _Value);

  void SetKeyInterpolation(TTimelineId _KeyId, ETimelineInterpolation _Interpolation);

  void PreviewMoveItems(const std::vector<TTimelineId> & _ItemIds, TTimelineTicks _Delta);

  void CommitPreview();

  void CancelPreview();

public: // Signals

  TChangedSignal & GetChangedSignal() noexcept;

protected: // Service

  void ResetTimeline(CTimeline _Timeline);

private: // Types

  using TEdit      = std::function<void(CTimelineDocument &)>;
  using TTrackEdit = std::function<void(TTimelineTrack &)>;

  struct TPreview
  {
    std::string                 CommandName;
    std::vector<TTimelineId>    ItemIds;
    std::vector<TTimelineTrack> Before;
  };

private: // Implementation

  void ExecuteCommand(std::string _CommandName, TEdit _Execute, TEdit _Undo);

  void EditTracks(std::string _CommandName, const std::vector<TTimelineId> & _TrackIds, const TTrackEdit & _Edit);

  void EditItemTrack(std::string _CommandName, TTimelineId _ItemId, const TTrackEdit & _Edit);

  std::vector<TTimelineId> FindItemTracks(const std::vector<TTimelineId> & _ItemIds) const;

  std::vector<TTimelineTrack> CaptureContents(const std::vector<TTimelineId> & _TrackIds) const;

  void ApplyContents(const std::vector<TTimelineTrack> & _Contents);

  void AssignIds(TTimelineTrack & _Track);

  void Notify(ETimelineChangeKind _Kind, TTimelineId _TrackId = InvalidTimelineId) const;

private: // Attributes

  CTimeline               m_Timeline;
  std::optional<TPreview> m_Preview;
  TChangedSignal          m_ChangedSignal;
};

} // namespace editor
