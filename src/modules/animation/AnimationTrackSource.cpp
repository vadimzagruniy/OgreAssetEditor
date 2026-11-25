#include "AnimationTrackSource.h"
#include "AnimationSlots.h"
#include "AnimationTracks.h"
#include "editor/inspector/PropertySchema.h"
#include "editor/timeline/model/Timeline.h"
#include "modules/scene/animation/ISceneAnimationHost.h"
#include "modules/scene/model/SceneGraph.h"
#include "modules/scene/model/SceneNode.h"
#include "modules/scene/model/SceneNodeTypes.h"

#include <string>
#include <utility>

namespace editor
{

namespace
{

bool HasTrack(const std::vector<TTimelineTrack> & _Tracks, const TTimelineTarget & _Target)
{
  for (const auto & Track : _Tracks)
  {
    if (Track.Target == _Target || HasTrack(Track.Children, _Target))
      return true;
  }

  return false;
}

void CollectTemplates(const std::vector<TPropertyDesc> & _Properties, const std::string & _GroupLabel, const std::string & _Slot, const std::string & _NodeName, const CTimeline & _Timeline, std::vector<TTimelineTrackTemplate> & _Templates)
{
  for (const auto & Property : _Properties)
  {
    if (Property.Type == EPropertyType::Group)
    {
      CollectTemplates(Property.Children, Property.Label, _Slot, _NodeName, _Timeline, _Templates);
      continue;
    }

    if (!Property.Meta.IsAnimatable)
      continue;

    TTimelineTrack Track;
    Track.TypeId = NodePropertyTrackType;
    Track.Name   = _NodeName + " " + Property.Label;
    Track.Target = {_Slot, Property.Identifier};

    if (HasTrack(_Timeline.GetTracks(), Track.Target))
      continue;

    const auto Label = _GroupLabel.empty() ? Property.Label : _GroupLabel + " / " + Property.Label;
    _Templates.push_back({Label, std::move(Track)});
  }
}

} // namespace

//
// Constructor
//

CAnimationTrackSource::CAnimationTrackSource(std::shared_ptr<const CAnimationSlots> _Slots):
  m_Slots(std::move(_Slots))
{
  // Empty
}

//
// ITimelineTrackSource
//

std::vector<TTimelineTrackTemplate> CAnimationTrackSource::GetTrackTemplates(const CTimeline & _Timeline) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host)
    return {};

  const auto   NodeName = Host->GetSelectedNode();
  const auto & Graph    = Host->GetGraph();

  if (NodeName.empty() || !Graph.HasNode(NodeName))
    return {};

  const auto & Schema = Graph.GetNodeTypes().GetSchema(std::string(Graph.GetNode(NodeName).GetTypeId()));
  const auto   Slot   = Host->FindSlot(m_Slots->GetAnimationPath(), NodeName);

  std::vector<TTimelineTrackTemplate> Templates;
  CollectTemplates(Schema.Properties, {}, Slot, NodeName, _Timeline, Templates);

  return Templates;
}

TPropertyValue CAnimationTrackSource::CaptureValue(const TTimelineTrack & _Track) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Track.TypeId != NodePropertyTrackType)
    return {};

  const auto & Graph    = Host->GetGraph();
  const auto   NodeName = m_Slots->ResolveSlot(_Track.Target.Slot);

  if (!Graph.HasNode(NodeName))
    return {};

  return Graph.GetNode(NodeName).GetValue(_Track.Target.Property);
}

bool CAnimationTrackSource::IsTargetValid(const TTimelineTrack & _Track) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Track.TypeId != NodePropertyTrackType)
    return true;

  return Host->GetGraph().HasNode(m_Slots->ResolveSlot(_Track.Target.Slot));
}

bool CAnimationTrackSource::CanBindToSelection(const TTimelineTrack & _Track) const
{
  const auto Host = m_Slots->LockHost();
  if (!Host || _Track.TypeId != NodePropertyTrackType || _Track.Target.Slot.empty())
    return false;

  const auto NodeName = Host->GetSelectedNode();
  return !NodeName.empty() && Host->HasAnimation(m_Slots->GetAnimationPath()) && m_Slots->ResolveSlot(_Track.Target.Slot) != NodeName;
}

void CAnimationTrackSource::BindToSelection(const TTimelineTrack & _Track) const
{
  if (!CanBindToSelection(_Track))
    return;

  const auto Host = m_Slots->LockHost();
  Host->BindSlot(m_Slots->GetAnimationPath(), _Track.Target.Slot, Host->GetSelectedNode());
}

} // namespace editor
