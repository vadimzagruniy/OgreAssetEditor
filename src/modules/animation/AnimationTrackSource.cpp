#include "AnimationTrackSource.h"
#include "AnimationTracks.h"
#include "editor/timeline/model/Timeline.h"
#include "modules/scene/animation/ISceneAnimationHost.h"
#include "modules/scene/model/SceneGraph.h"
#include "modules/scene/model/SceneNode.h"

#include <array>
#include <string>
#include <utility>

namespace editor
{

namespace
{

constexpr std::array<std::pair<std::string_view, std::string_view>, 3> TransformProperties
{{
  {PositionProperty,    "Position"},
  {OrientationProperty, "Rotation"},
  {ScaleProperty,       "Scale"   }
}};

bool HasTrack(const std::vector<TTimelineTrack> & _Tracks, const TTimelineTarget & _Target)
{
  for (const auto & Track : _Tracks)
  {
    if (Track.Target == _Target || HasTrack(Track.Children, _Target))
      return true;
  }

  return false;
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

  const auto NodeName = Host->GetSelectedNode();
  if (NodeName.empty())
    return {};

  std::vector<TTimelineTrackTemplate> Templates;

  for (const auto & [Identifier, Label] : TransformProperties)
  {
    TTimelineTrack Track;
    Track.TypeId = TransformTrackType;
    Track.Name   = NodeName + " " + std::string(Label);
    Track.Target = {NodeName, std::string(Identifier)};

    if (!HasTrack(_Timeline.GetTracks(), Track.Target))
      Templates.push_back({Track.Name, std::move(Track)});
  }

  return Templates;
}

TPropertyValue CAnimationTrackSource::CaptureValue(const TTimelineTrack & _Track) const
{
  const auto Host = m_Host.lock();
  if (!Host || _Track.TypeId != TransformTrackType)
    return {};

  const auto & Graph = Host->GetGraph();
  if (!Graph.HasNode(_Track.Target.Slot))
    return {};

  const auto & Transform = Graph.GetNode(_Track.Target.Slot).GetTransform();
  const auto & Property  = _Track.Target.Property;

  if (Property == PositionProperty)
    return Transform.Position;

  if (Property == OrientationProperty)
    return Transform.Orientation;

  if (Property == ScaleProperty)
    return Transform.Scale;

  return {};
}

} // namespace editor
