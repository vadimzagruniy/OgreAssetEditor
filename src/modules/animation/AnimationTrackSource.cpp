#include "AnimationTrackSource.h"
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

void CollectTemplates(const std::vector<TPropertyDesc> & _Properties, const std::string & _GroupLabel, const std::string & _NodeName, const CTimeline & _Timeline, std::vector<TTimelineTrackTemplate> & _Templates)
{
  for (const auto & Property : _Properties)
  {
    if (Property.Type == EPropertyType::Group)
    {
      CollectTemplates(Property.Children, Property.Label, _NodeName, _Timeline, _Templates);
      continue;
    }

    if (!Property.Meta.IsAnimatable)
      continue;

    TTimelineTrack Track;
    Track.TypeId = NodePropertyTrackType;
    Track.Name   = _NodeName + " " + Property.Label;
    Track.Target = {_NodeName, Property.Identifier};

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

CAnimationTrackSource::CAnimationTrackSource(std::weak_ptr<ISceneAnimationHost> _Host):
  m_Host(std::move(_Host))
{
  // Empty
}

//
// ITimelineTrackSource
//

std::vector<TTimelineTrackTemplate> CAnimationTrackSource::GetTrackTemplates(const CTimeline & _Timeline) const
{
  const auto Host = m_Host.lock();
  if (!Host)
    return {};

  const auto   NodeName = Host->GetSelectedNode();
  const auto & Graph    = Host->GetGraph();

  if (NodeName.empty() || !Graph.HasNode(NodeName))
    return {};

  const auto & Schema = Graph.GetNodeTypes().GetSchema(std::string(Graph.GetNode(NodeName).GetTypeId()));

  std::vector<TTimelineTrackTemplate> Templates;
  CollectTemplates(Schema.Properties, {}, NodeName, _Timeline, Templates);

  return Templates;
}

TPropertyValue CAnimationTrackSource::CaptureValue(const TTimelineTrack & _Track) const
{
  const auto Host = m_Host.lock();
  if (!Host || _Track.TypeId != NodePropertyTrackType)
    return {};

  const auto & Graph = Host->GetGraph();
  if (!Graph.HasNode(_Track.Target.Slot))
    return {};

  return Graph.GetNode(_Track.Target.Slot).GetValue(_Track.Target.Property);
}

} // namespace editor
